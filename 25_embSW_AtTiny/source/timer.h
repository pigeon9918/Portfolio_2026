#ifndef __TIMER_H__
#define __TIMER_H__

#include "pinMap.h"

#include <avr/io.h>
#include <avr/interrupt.h>

extern volatile uint32_t sys_tick;

ISR(TIMER0_OVF_vect);
void timer0_init(void);
void timer1_init(void);
void pwm_init();
void tone(uint16_t *scale, uint8_t octave, uint8_t note);
void set_pwm_duty(uint8_t duty);

inline uint8_t get_pwm_duty() {
    //// OCR0B를 0~255 사이의 값으로 설정 (값이 작을수록 PWM 출력이 High인 시간이 짧음).
    return OCR0B;
}

#endif