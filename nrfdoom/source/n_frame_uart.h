#ifndef N_FRAME_UART_H
#define N_FRAME_UART_H

#include <stdint.h>
#include <stddef.h>

// One-way (TX only) high-baud UART used to stream rendered frames off the
// board for the ground-station prototype. Independent of the UARTE1
// instance in n_uart.c, which is already shared by debug printf and
// gamepad/keyboard input.

void N_frame_uart_init(void);

// Sends one DOOM_FRAME packet: PLAYPAL index byte + RLE-encoded pixels.
// pixels must be width*height bytes of 8-bit palette indices.
void N_frame_uart_send_frame(const uint8_t *pixels, size_t width, size_t height, uint8_t playpal_index);

#endif
