#ifndef N_SPI_H
#define N_SPI_H
#include <stdbool.h>
#include <stdint.h>

struct packet {
  uint8_t len;
  uint8_t sequence[246];
};

typedef enum{
  TX_BUFFER_IDLE = 0, 
  TX_BUFFER_LOADED = 1,
  TX_BUFFER_ARMED = 2,
} tx_buffer_state_t;

void spi_init(void);
bool spi_has_data(void);
bool spi_get_pkt(struct packet *output_pkt);
bool spi_put_pkt(const struct packet *input_pkt);

#endif
