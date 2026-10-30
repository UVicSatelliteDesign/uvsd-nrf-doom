#ifndef N_SPI_H
#define N_SPI_H
#include <stdbool.h>

#define SPI_CMD_RETRIEVAL_FAILURE 0x15

void spi_init();
bool spi_has_data();
char spi_getc();
void spi_putc(char ch);

#endif
