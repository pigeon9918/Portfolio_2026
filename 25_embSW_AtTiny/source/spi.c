#include <avr/io.h>
#include "pinMap.h"
#include <util/delay.h>

#include "spi.h"

// USI SPI 1-byte 출력 함수
void SpiUSITx(uint8_t data) {
    USICR |= 0b00010000;
    USIDR = data;
    for (uint8_t i=0; i<8; i++) {
        USICR = 0b00010001;
        USICR = 0b00010011;
    }
    USICR &= ~(0b00110000);
}