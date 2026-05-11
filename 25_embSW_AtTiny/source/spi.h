#ifndef __SPI_H__
#define __SPI_H__

#include <avr/io.h>
#include "pinMap.h"

// USI SPI 1-byte
void SpiUSITx(uint8_t data);

#endif