#include "pinMap.h"
#include "timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>

ISR(TIMER0_OVF_vect) {
    ++sys_tick;
}

void timer0_init(void) {
    //// TCCR0A/B 의 WGM을 Fast PWM, TOP=0xFF로 설정.
    TCCR0A |= 0b11 << WGM00;
    TCCR0B |= 0b0 << WGM02;
    // 3. 인터럽트 활성화 (Overflow Interrupt)
    // TCNT1이 MAX(TOP)에 도달하여 0으로 넘어가는 순간 발생
    TIMSK |= (1 << TOIE0);
    TCCR0B |= 0b011 << CS00; // Divide by 64, start counting
}

void timer1_init(void) {
    //// WGM1 값을 CTC Mode (TOP = OCR1A)로 설정
    TCCR1A |= 0b00 << WGM10;
    TCCR1B |= 0b01 << WGM12;
    //// COM1A 값을 Toggle로 설정
    TCCR1A |= 0b01 << COM1A0;
    OCR1A = 0xFFFF;
    //// Clock source와 Pre-scale 값을 설정하여 카운트 시작
    TCCR1B |= 0b011 << CS00;
}

void tone(uint16_t *scale, uint8_t octave, uint8_t note) {
    if (octave == 0 || octave > 7  || note < 0 || note > 6) {
        // 1. 타이머와 핀의 연결 해제 (Toggle 모드 해제)
        TCCR1A &= ~(0b11 << COM1A0); 
        // 2. 핀의 상태를 강제로 Low로 설정 (잔류 전압 제거)
        OC1A_PORT &= ~(1 << OC1A_BIT);
    } 
    else {
        // 소리를 내야 하는 경우
        TCNT1 = 0;
        TCCR1A |= 0b01 << COM1A0;
        OC1A_PORT |= (1 <<OC1A_BIT);
        OCR1A = ((F_CPU >> 7) / (scale[note] >> (7 - octave))) - 1 ;
    }
}

void pwm_init() {    
    //// PD5의 Data Direction Bit를 Output으로 설정.
    DDRD |= 1 << OC0B_BIT;
    //// OCR0B를 0~255 사이의 값으로 설정 (값이 작을수록 PWM 출력이 High인 시간이 짧음).
    OCR0B = 0;
    //// TCCR0A의 COM0B를 non-inverting mode (Clear OC0B on Compare Match)로 설정.
    TCCR0A |= 0b10 << COM0B0;
    // F_CPU ÷ 64 ÷ 256 Hz 로 LED 가 켜지고 꺼짐(다른 주파수도 사용 가능).
    // PWM 출력이 시작됨 (CS에 0 이 아닌 값이 write 되면 Timer 0 시작).
}

inline void set_pwm_duty(uint8_t duty) {
    if (duty != 0) {
        TCCR0A |= 0b10 << COM0B0;
    }
    else {
        TCCR0A &= ~(0b11 << COM0B0);
    }
    //// OCR0B를 0~255 사이의 값으로 설정 (값이 작을수록 PWM 출력이 High인 시간이 짧음).
    OCR0B = duty;
}