#include "uart.h"
#include "pinMap.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>
#include <avr/pgmspace.h>

ISR(USART0_RX_vect) {
    //// UDR의 값을 g_rxByte 전역 변수에 저장.
    g_rxByte = UDR;
}

void uart_init(void) {
    UBRRH = UBRR_VALUE >> 8 ;
    UBRRL = UBRR_VALUE;
    //// UCSRB의 TXEN과 RXEN를 활성화
    UCSRB = (1<<RXEN)|(1<<TXEN);
    //// UCSRC를 Async, 8 data, 1 stop, no parity로 설정.
    UCSRC = (0b11<<UCSZ0);
    //// USART0_RX 인터럽트 활성화 추가
	UCSRB |= (1<<RXCIE); //RXC 인터럽트 활성화
}

void uart_tx(uint8_t data) {
    /* Wait for empty transmit buffer */
    while ( !( UCSRA & (1<<UDRE)) ) {}
    /* Put data into buffer, sends the data */
    UDR = data;
}

void uart_tx_str(char * str) {
    while (*str) {
        uart_tx(*str++);
    }
}

void uart_tx_str_pgm(const char * str) {
    while (pgm_read_byte(str)) {
        uart_tx(pgm_read_byte(str++));
    }
}

void uart_tx_uint32_text(uint32_t data) {
    char buf[11]; // uint32_t 최대값은 4,294,967,295로 10자리 + 널문자
    uint8_t i = 0;

    if (data == 0) {
        uart_tx('0');
        return;
    }

    // 일의 자리부터 추출하여 버퍼에 저장
    while (data > 0) {
        buf[i++] = (data % 10) + '0';
        data /= 10;
    }

    // 버퍼에는 숫자가 역순으로 담겨 있으므로 거꾸로 출력
    while (i > 0) {
        uart_tx(buf[--i]);
    }
}