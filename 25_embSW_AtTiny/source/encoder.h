#ifndef __ENCODER_H__
#define __ENCODER_H__

#include <avr/io.h>
#include <avr/interrupt.h>

#define MAX_COUNT   511

extern volatile int16_t counter;

void INT0_ENC_init();
ISR(INT0_vect);

#endif