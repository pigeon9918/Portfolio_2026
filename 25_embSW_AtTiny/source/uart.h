#ifndef __UART_H__
#define __UART_H__

#include "pinMap.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>

extern volatile uint8_t g_rxByte;

ISR(USART0_RX_vect);
void uart_init(void);
void uart_tx(uint8_t data);
void uart_tx_str(char * str);
void uart_tx_str_pgm(const char * str);
void uart_tx_uint32_text(uint32_t data);

#endif