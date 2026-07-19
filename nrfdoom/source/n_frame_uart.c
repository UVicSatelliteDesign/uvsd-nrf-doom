#include "n_frame_uart.h"

#include "nrf.h"
#include "board_config.h"

// This uses UART right now.
// UVSD-TODO: split into an encoding/checksum file (packet framing, RLE,
// CRC16) and a stream file/interface that the encoding code calls into, so
// that stream file can be swapped for UART or SPI.
//
// Packet format: SYNC(2B) | TYPE(1B) | LENGTH(2B LE) | PAYLOAD(NB) | CRC16(2B LE)
// PAYLOAD for a DOOM_FRAME packet is: PLAYPAL index(1B) | RLE(pixels)
// CRC16 (CCITT-FALSE) covers TYPE, LENGTH and PAYLOAD.
//
// UVSD-TODO: only the FRAME packet type is implemented. PALETTE, GAMESTATE,
// INPUT, and HEARTBEAT from the planned protocol don't exist yet.
#define FRAME_SYNC0 0xAA
#define FRAME_SYNC1 0x55
#define FRAME_TYPE_DOOM_FRAME 0x02

static volatile char frame_uart_tx_buf[1];

void N_frame_uart_init(void)
{
    NRF_FRAME_UARTE->PSEL.TXD = UART0_TX_PIN;
    NRF_FRAME_UARTE->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud1M;
    NRF_FRAME_UARTE->CONFIG = 0;
    NRF_FRAME_UARTE->ENABLE = UARTE_ENABLE_ENABLE_Enabled;
    NRF_FRAME_UARTE->EVENTS_ENDTX = 0;
}

// UVSD-TODO: blocks until each byte is sent, so N_frame_uart_send_frame
// stalls the render loop for the whole frame. Fine for UART; revisit with a
// non-blocking/DMA-buffered send when SPI's data-ready handshake is added.
static void N_frame_uart_putc(uint8_t ch)
{
    NRF_FRAME_UARTE->EVENTS_ENDTX = 0;

    frame_uart_tx_buf[0] = ch;
    NRF_FRAME_UARTE->TXD.PTR = (uint32_t)(&frame_uart_tx_buf[0]);
    NRF_FRAME_UARTE->TXD.MAXCNT = 1;

    NRF_FRAME_UARTE->TASKS_STARTTX = 1;

    while (!NRF_FRAME_UARTE->EVENTS_ENDTX) {}
}

static uint16_t crc16_update(uint16_t crc, uint8_t data)
{
    crc ^= (uint16_t)data << 8;
    for (int i = 0; i < 8; i++)
    {
        if (crc & 0x8000)
        {
            crc = (crc << 1) ^ 0x1021;
        }
        else
        {
            crc <<= 1;
        }
    }
    return crc;
}

// Length of the run starting at pixels[i] (capped at 255, the max a single
// RLE count byte can hold).
static size_t rle_run_length(const uint8_t *pixels, size_t i, size_t len)
{
    uint8_t value = pixels[i];
    size_t run = 1;
    while (i + run < len && pixels[i + run] == value && run < 255)
    {
        run++;
    }
    return run;
}

// Counts the encoded size without emitting anything, so LENGTH can be sent
// before PAYLOAD without buffering the whole RLE-encoded frame in RAM.
static size_t rle_encoded_size(const uint8_t *pixels, size_t len)
{
    size_t encoded = 0;
    size_t i = 0;
    while (i < len)
    {
        i += rle_run_length(pixels, i, len);
        encoded += 2;
    }
    return encoded;
}

static void send_byte(uint8_t b, uint16_t *crc)
{
    N_frame_uart_putc(b);
    *crc = crc16_update(*crc, b);
}

void N_frame_uart_send_frame(const uint8_t *pixels, size_t width, size_t height, uint8_t playpal_index)
{
    size_t npixels = width * height;
    uint16_t payload_len = (uint16_t)(1 + rle_encoded_size(pixels, npixels));
    uint16_t crc = 0xFFFF;

    N_frame_uart_putc(FRAME_SYNC0);
    N_frame_uart_putc(FRAME_SYNC1);

    send_byte(FRAME_TYPE_DOOM_FRAME, &crc);
    send_byte(payload_len & 0xFF, &crc);
    send_byte((payload_len >> 8) & 0xFF, &crc);

    send_byte(playpal_index, &crc);

    size_t i = 0;
    while (i < npixels)
    {
        size_t run = rle_run_length(pixels, i, npixels);
        send_byte((uint8_t)run, &crc);
        send_byte(pixels[i], &crc);
        i += run;
    }

    N_frame_uart_putc(crc & 0xFF);
    N_frame_uart_putc((crc >> 8) & 0xFF);
}
