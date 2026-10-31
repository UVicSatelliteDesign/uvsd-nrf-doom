#ifndef N_SPI_H
#define N_SPI_H
#include <stdbool.h>

void spi_init(void);
bool spi_has_data(void);
bool spi_getc(char *output_val);
void spi_putc(char ch);

#endif
