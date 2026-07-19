#!/usr/bin/env python3
"""
Ground-station prototype viewer for the nRF5340 Doom frame UART.

Reads packets of the form:
    SYNC(2B: 0xAA,0x55) | TYPE(1B) | LENGTH(2B LE) | PAYLOAD(NB) | CRC16(2B LE)

The only packet type currently sent is DOOM_FRAME (0x02), whose payload is:
    PLAYPAL index(1B) | RLE-encoded 320x200 8-bit palette indices

All 14 PLAYPAL palettes are loaded once from doom1.wad so the wire format
only needs to carry a 1-byte palette index per frame (see nrfdoom/source/
n_frame_uart.c and doom/st_stuff.c:ST_GetPaletteIndex).

Usage:
    python3 frame_viewer.py --port /dev/ttyUSB0 --baud 1000000
"""

import argparse
import struct
import sys

import cv2
import numpy as np
import serial

SYNC0 = 0xAA
SYNC1 = 0x55
TYPE_DOOM_FRAME = 0x02

SCREEN_WIDTH = 320
SCREEN_HEIGHT = 200
NUM_PIXELS = SCREEN_WIDTH * SCREEN_HEIGHT


def crc16_update(crc, byte):
    crc ^= byte << 8
    for _ in range(8):
        if crc & 0x8000:
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF
        else:
            crc = (crc << 1) & 0xFFFF
    return crc


def crc16(data):
    crc = 0xFFFF
    for b in data:
        crc = crc16_update(crc, b)
    return crc


def load_playpal(wad_path):
    with open(wad_path, "rb") as f:
        data = f.read()
    _, numlumps, infotableofs = struct.unpack_from("<4sii", data, 0)
    for i in range(numlumps):
        filepos, size, name = struct.unpack_from("<ii8s", data, infotableofs + i * 16)
        if name.split(b"\x00")[0] == b"PLAYPAL":
            raw = data[filepos:filepos + size]
            return np.frombuffer(raw, dtype=np.uint8).reshape(-1, 256, 3)
    raise ValueError(f"No PLAYPAL lump found in {wad_path}")


def read_exact(ser, n):
    buf = bytearray()
    while len(buf) < n:
        chunk = ser.read(n - len(buf))
        if chunk:
            buf += chunk
    return bytes(buf)


def find_sync(ser):
    prev = 0
    while True:
        b = read_exact(ser, 1)[0]
        if prev == SYNC0 and b == SYNC1:
            return
        prev = b


def decode_rle(payload, expected_len):
    out = bytearray()
    i = 0
    while i + 1 < len(payload) and len(out) < expected_len:
        count = payload[i]
        value = payload[i + 1]
        out.extend([value] * count)
        i += 2
    return bytes(out[:expected_len])


def read_packet(ser):
    find_sync(ser)
    type_len = read_exact(ser, 3)
    ptype = type_len[0]
    length = struct.unpack_from("<H", type_len, 1)[0]
    payload = read_exact(ser, length)
    crc_bytes = read_exact(ser, 2)
    received_crc = struct.unpack_from("<H", crc_bytes, 0)[0]
    computed_crc = crc16(type_len + payload)
    if received_crc != computed_crc:
        print(f"CRC mismatch (type={ptype:#x} len={length}), resyncing", file=sys.stderr)
        return None
    return ptype, payload


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True, help="Serial port, e.g. /dev/ttyUSB0")
    parser.add_argument("--baud", type=int, default=1_000_000)
    parser.add_argument("--wad", default="doom1.wad", help="Path to doom1.wad (for PLAYPAL)")
    args = parser.parse_args()

    playpal = load_playpal(args.wad)
    print(f"Loaded {playpal.shape[0]} PLAYPAL palettes from {args.wad}")

    ser = serial.Serial(args.port, args.baud)

    cv2.namedWindow("nRF5340 Doom", cv2.WINDOW_NORMAL)

    while True:
        packet = read_packet(ser)
        if packet is None:
            continue
        ptype, payload = packet
        if ptype != TYPE_DOOM_FRAME:
            print(f"Unknown packet type {ptype:#x}, skipping", file=sys.stderr)
            continue

        playpal_index = payload[0]
        indices = decode_rle(payload[1:], NUM_PIXELS)
        if len(indices) != NUM_PIXELS:
            print(f"Short frame ({len(indices)}/{NUM_PIXELS} pixels), skipping", file=sys.stderr)
            continue

        pal = playpal[playpal_index % playpal.shape[0]]
        idx_image = np.frombuffer(indices, dtype=np.uint8).reshape(SCREEN_HEIGHT, SCREEN_WIDTH)
        rgb_image = pal[idx_image]
        bgr_image = cv2.cvtColor(rgb_image, cv2.COLOR_RGB2BGR)

        cv2.imshow("nRF5340 Doom", bgr_image)
        if cv2.waitKey(1) == ord("q"):
            break


if __name__ == "__main__":
    main()
