#ifndef N_SPI_H
#define N_SPI_H
#include <stdbool.h>

void spi_init();
bool spi_has_data();
char spi_getc();
void spi_putc(char ch);

#endif
