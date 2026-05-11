#include "encoder.h"
#include "pinMap.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static volatile uint8_t a0 = 0;

void INT0_ENC_init() {
    //// A핀과 B핀을 입력으로 설정 (초기 값이 0이므로 생략 가능)
    ENC_CLK_DDR &= ~(1 << ENC_CLK_BIT);
    ENC_DT_DDR &= ~(1 << ENC_DT_BIT);
    //// A핀과 B핀의 Pull-Up 저항 활성화
    ENC_CLK_PORT |= (1 << ENC_CLK_BIT);
    ENC_DT_PORT |= (1 << ENC_DT_BIT);
    //// INT0를 all edge에서 발생하도록 MCUCR 설정.
    MCUCR &= ~(0b11 << ISC00);
    MCUCR |= (0b01 << ISC00);
    //// INT0를 활성화 하도록 GIMSK 설정.
    GIMSK |= (1 << INT0);
}

ISR(INT0_vect) {
    //// A핀의 값을 읽음
    uint8_t a = (ENC_CLK_PIN >> ENC_CLK_BIT) & 1;

    if(a != a0) {
        //// B핀의 값을 읽음
        uint8_t b = (ENC_DT_PIN >> ENC_DT_BIT) & 1;
        
        if(a != b) {
            //// A와 B 값이 다르면 counter를 증가;
            counter++;
        }
        else {
            //// 그렇지 않으면 counter를 감소;
            counter--;
        }
        a0 = a;
    }

    //// counter가 MAX_COUNT보다 커지면 0으로 초기화
    counter = (counter > MAX_COUNT) ? 0 : counter;
    //// counter가 0보다 작아지면 MAX_COUNT로 초기화
    counter = (counter < 0) ? MAX_COUNT : counter;
}
