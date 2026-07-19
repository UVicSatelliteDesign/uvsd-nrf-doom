
#include "nrf_gpio.h"

#define NRF_UARTE  NRF_UARTE1_S

// Second, TX-only UART used to stream rendered frames to a ground-station
// receiver. UARTE1 above is already shared by debug printf and gamepad
// input. UARTE2 (SERIAL2) is used because on the nRF5340 UARTE0 shares one
// serial block (SERIAL0) with SPIM0, which the SD card driver needs
// (APP_SDCARD_SPI_INSTANCE 0) — enabling both hangs SD init. P1.01 is the TXD line of
// the DK interface MCU's second virtual COM port (VCOM1; VCOM0 = P0.20/P0.22
// is the debug console), so frames arrive over the J-Link USB cable with no
// external wiring. For an external bridge (Pi Pico OBC), switch back to a
// header pin such as P0.10 (free GPIO: NFC is disabled on this board).
#define NRF_FRAME_UARTE NRF_UARTE2_S
#define UART0_TX_PIN NRF_GPIO_PIN_MAP(1, 1)
// Previous mapping — header pin for an external bridge (FT232H, Pi Pico):
// #define UART0_TX_PIN NRF_GPIO_PIN_MAP(0, 10)

// Display
#define NRF_DISPLAY_SPIM   NRF_SPIM4_S
#define NRF_DISPLAY_GPIOTE NRF_GPIOTE0_S

#define NRF_DOOM_TIMER NRF_TIMER0_S

#define BUTTON_PIN_1 23
#define BUTTON_PIN_2 24
#define BUTTON_PIN_3 8
#define BUTTON_PIN_4 9

#define LED_PIN_1 28
#define LED_PIN_2 29
#define LED_PIN_3 30
#define LED_PIN_4 31

#define UART_TX_PIN 20
#define UART_RX_PIN 22

#define QSPI_SCK_PIN 17
#define QSPI_CSN_PIN 18
#define QSPI_IO0_PIN 13
#define QSPI_IO1_PIN 14
#define QSPI_IO2_PIN 15
#define QSPI_IO3_PIN 16


#define SDC_SCK_PIN     NRF_GPIO_PIN_MAP(1, 14)
#define SDC_MOSI_PIN    NRF_GPIO_PIN_MAP(1, 13)
#define SDC_MISO_PIN    NRF_GPIO_PIN_MAP(1, 15)
#define SDC_CS_PIN      NRF_GPIO_PIN_MAP(1, 12)

#define DISPLAY_PIN_SCK  NRF_GPIO_PIN_MAP(0, 6)
#define DISPLAY_PIN_MISO NRF_GPIO_PIN_MAP(0, 5)
#define DISPLAY_PIN_MOSI NRF_GPIO_PIN_MAP(0, 25)
#define DISPLAY_PIN_CS_N NRF_GPIO_PIN_MAP(0, 7)
#define DISPLAY_PIN_PD_N 26

// #define MAX98357
#define PCM5102

#ifdef PCM5102
#define I2S_PIN_SCK     NRF_GPIO_PIN_MAP(1, 9)
#define I2S_PIN_BCK     NRF_GPIO_PIN_MAP(1, 8)
#define I2S_PIN_DIN     NRF_GPIO_PIN_MAP(1, 7)
#define I2S_PIN_LRCK    NRF_GPIO_PIN_MAP(1, 6)
#endif

#ifdef MAX98357
#define I2S_PIN_SD      NRF_GPIO_PIN_MAP(0, 10);
#define I2S_PIN_GAIN    NRF_GPIO_PIN_MAP(0, 9);
#define I2S_PIN_DIN      NRF_GPIO_PIN_MAP(1, 0);
#define I2S_PIN_BCK      NRF_GPIO_PIN_MAP(0, 24); // BCLK
#define I2S_PIN_LRCK     NRF_GPIO_PIN_MAP(0, 22); // LRC
#endif