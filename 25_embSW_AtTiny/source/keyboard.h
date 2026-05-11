#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "pinMap.h"

#include <avr/io.h>
#include <avr/interrupt.h>

extern volatile uint16_t scanCode;

void INT1_KB_init();
ISR(INT1_vect);
uint8_t ps2_scan_to_ascii(uint16_t code);

#endif