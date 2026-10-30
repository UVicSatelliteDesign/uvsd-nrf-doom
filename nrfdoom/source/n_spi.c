#include "n_spi.h"
#include "hal/nrf_gpio.h"
#include "nrfx_spis.h"
#include "ring_buffer.h"
#include <stdbool.h>
#define TX_BUF_LENGTH 32
#define RX_BUF_LENGTH 32

static uint8_t tx_buffer[TX_BUF_LENGTH];
static uint8_t rx_buffer[RX_BUF_LENGTH];
static ring_buffer_t tx_ring_buf;
static ring_buffer_t rx_ring_buf;
nrfx_spis_t nrfx_spi = NRFX_SPIS_INSTANCE(0);

void spi_event_handler(nrfx_spis_evt_t const *p_event, void *p_context) {
    // SPI transaction event. SPI transaction has been completed.
    if (p_event->evt_type == NRFX_SPIS_XFER_DONE) {
        int index;
        // Cycle through all newly added bytes in rx_buffer and move them to the rx ring buffer
        for (index = 0; index < p_event->rx_amount; index++) {
            ring_buffer_add(&rx_ring_buf, rx_buffer[index]);
            rx_buffer[index] = 0;
        }
        index = 0;
        // Move bytes from the tx ring buf to the tx buf (staging buf) til tx is full or ring empty
        while (index < TX_BUF_LENGTH) {
            if (!ring_buffer_pop(&tx_ring_buf, &tx_buffer[index]))
                tx_buffer[index] = 0x00; // Fill remaining tx_buffer with empty bytes
            index++;
        }
        // Arm the staging buffers for the next transfer.
        nrfx_spis_buffers_set(&nrfx_spi, tx_buffer, TX_BUF_LENGTH, rx_buffer, RX_BUF_LENGTH);
    }
}

void spi_init(void) {
    nrfx_spis_config_t nrfx_spis_config = {
        .miso_pin = NRF_GPIO_PIN_MAP(1, 14),
        .mosi_pin = NRF_GPIO_PIN_MAP(1, 13),
        .sck_pin = NRF_GPIO_PIN_MAP(1, 15),
        .csn_pin = NRF_GPIO_PIN_MAP(1, 12),
        .mode = NRF_SPIS_MODE_0, // m0: SCK active high, sample on leading edge of the clock signal)
        .bit_order = NRF_SPIS_BIT_ORDER_MSB_FIRST,
        .csn_pullup = NRF_GPIO_PIN_NOPULL, // CSN pull-up disabled
        .miso_drive = NRF_GPIO_PIN_S0S1,   // MISO pin drive set to standard '0' and standard '1'
        .def = 0xFF,                       // default character set to 0xFF
        .orc = 0xFE,                       // over-read character set to 0xFE
        .irq_priority = NRFX_SPIS_DEFAULT_CONFIG_IRQ_PRIORITY, // Interrupt priority level
    };

    init_ring_buffer(&tx_ring_buf);
    init_ring_buffer(&rx_ring_buf);

    // Add pointer to additional context of any type if necessary in future
    nrfx_err_t spi_init_result =
        nrfx_spis_init(&nrfx_spi, &nrfx_spis_config, spi_event_handler, NULL);
    nrfx_err_t spi_buf_set_result =
        nrfx_spis_buffers_set(&nrfx_spi, tx_buffer, TX_BUF_LENGTH, rx_buffer, RX_BUF_LENGTH);
    if (spi_buf_set_result != NRFX_SUCCESS || spi_init_result != NRFX_SUCCESS) {
        // Flag SPI initialization error
    }
}

bool spi_has_data(void) { return !ring_buffer_is_empty(&rx_ring_buf); }

char spi_getc() {
    uint8_t byte;
    bool pop_success = false;
    pop_success = ring_buffer_pop(&rx_ring_buf, &byte);
    if (pop_success)
        return (char)byte;
    else
        return SPI_CMD_RETRIEVAL_FAILURE; // Warning value resenting a failure to get a command
                                          // (i.e. buffer is empty)
}

// If buffer is full there will be no effect
void spi_putc(char ch) { ring_buffer_add(&tx_ring_buf, ch); }
