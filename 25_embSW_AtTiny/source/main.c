/*
 * ESW_project_02.c
 *
 * Created: 2026-01-09 오전 11:58:49
 * Author : ehdwn
 * chip : ATtiny4313
 */ 


#define F_CPU       16000000
#define BAUD        38400

#define DEBUG_MODE  1

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>
#include <avr/pgmspace.h>

const char STR_ADMIN_CONSOLE_0[] PROGMEM = "\r\n == Admin Console ==\r\n>current states\r\n";

#include "pinMap.h"
#include "oled.h"
#include "spi.h"
#include "keyboard.h"
#include "timer.h"
#include "uart.h"
#include "encoder.h"

volatile uint32_t   sys_tick = 0;
volatile int16_t    counter = 0;
volatile uint16_t   scanCode = 0;
volatile uint8_t    g_rxByte = 0;

#define TICKS_TO_MS(t)    ((uint32_t)(((uint32_t)(t) * 128UL) / 125UL))
#define MS_TO_TICKS(m)    ((uint32_t)(((uint32_t)(m) * 125UL) / 128UL))

uint16_t scale[] = {
    2093,
    2349,
    2637,
    2794,
    3126,
    3520,
    3951
};

int main(void) {
    
    timer0_init();
    LED_DDR |= 1 << LED_BIT; // Set LED pin as output
    pwm_init(0);
    set_pwm_duty(0);
    
    timer1_init();
    // OC1A Pin을 출력으로 설정
    BUZ_DDR |= 1 << BUZ_BIT;
    tone(scale, 0, 0);
    
    // set DDR output for USI
    DO_DDR  |= (1 << DO_BIT);
    SCK_DDR |= (1 << SCK_BIT);

    ENC_SW_DDR &= ~(1 << ENC_SW_BIT);
    ENC_SW_PORT |= 1 << ENC_SW_BIT;
    
    sh1106_init();
    sh1106_clear();
    
    // sh1106_border();
    sh1106_draw_guage_border();
    sh1106_draw_bulb(0, 16 + 2);
    
    INT0_ENC_init();
    INT1_KB_init();
    
    uart_init();
    
    // 전체 인터럽트 활성화
    sei();
    
    uint32_t prev_toggle_tick = 0;
    uint32_t monitor_update_tick = 0;
    uint32_t last_switch_tick = 0;
    uint32_t tone_off_tick = 0;
    uint32_t last_encoder_update_tick = 0;

    int16_t prev_counter = 0;
    uint8_t last_sw_state = 1;
    uint8_t prev_Byte = 0;
    uint8_t prev_Byte_2 = 0;

    uint8_t octave = 0;
    uint8_t note = 0;

    while(1) {
        // system heart beat LED
        if (sys_tick - prev_toggle_tick >= MS_TO_TICKS(5000) ) {
            prev_toggle_tick = sys_tick;
            LED_PIN = 1 << LED_BIT;
            #if DEBUG_MODE
            uart_tx_str("\n\rLED Toggled!");
            #endif
        }
        // Encoder Handling
        if (counter != prev_counter) {
            last_encoder_update_tick = sys_tick;
            prev_counter = counter;
        }
        // counter timeout
        if(last_encoder_update_tick != 0 && sys_tick - last_encoder_update_tick >= MS_TO_TICKS(3000)) {
            last_encoder_update_tick = 0;
            prev_counter = counter = get_pwm_duty() * 2;
        }
        // update screen
        if (sys_tick - monitor_update_tick >= MS_TO_TICKS(50)) {
            monitor_update_tick = sys_tick;
            sh1106_fill_guage(counter / 4);
        }
        // counter click
        if(sys_tick - last_switch_tick >= 50) {
            last_switch_tick = sys_tick;
            
            // 현재 상태 읽기
            uint8_t current_sw_state = (ENC_SW_PIN & (1 << ENC_SW_BIT)) ? 1 : 0;

            // Falling Edge
            if (last_sw_state == 1 && current_sw_state == 0) {
                set_pwm_duty(counter / 2); // 스위치를 누르는 순간 현재 카운터 값을 PWM에 반영
                #if DEBUG_MODE
                uart_tx_str("Switch Clicked!\r\n");
                uart_tx_str("set duty: "); uart_tx('0' + (counter / 100)); uart_tx('0' + (counter % 100 / 10)); uart_tx('0' + (counter % 10)); uart_tx('\r'); uart_tx('\n');
                #endif
            }
            
            last_sw_state = current_sw_state; // 상태 업데이트
        }
        // tone
        if (tone_off_tick != 0) {
            if (sys_tick >= tone_off_tick) {
                tone(scale, 0, 0);
                tone_off_tick = 0;
            }
        }
        // KBD
        if (scanCode) {
            uint8_t ch = ps2_scan_to_ascii(scanCode);
            scanCode = 0;

            int16_t target_duty = -1;
            if (ch >= '0' && ch <= '9') {
                uint8_t n = ch - '0';
                target_duty = (n == 9) ? MAX_COUNT : (uint16_t)(MAX_COUNT * n) / 9;
                note = (n == 0) ? 2 : (n - 1) % 7;
                octave = (n >= 8 || n == 0) ? 7 : 6;
            }

            if (target_duty != -1) {
                prev_counter = counter = target_duty;
                tone(scale, octave, note);
                tone_off_tick = sys_tick + 250;
                set_pwm_duty(counter / 2);
            }
            
            #if DEBUG_MODE
            uart_tx(ch);
            #endif
        }
        // UART debugger
        if (g_rxByte) {
            uart_tx(g_rxByte);

            if (prev_Byte_2 == 'l' && prev_Byte == 's' && g_rxByte == '\r') {
                uart_tx_str_pgm(STR_ADMIN_CONSOLE_0);
                uint8_t buf = get_pwm_duty();
                uart_tx_str("ticks: "); uart_tx_uint32_text(sys_tick); uart_tx('\r'); uart_tx('\n');
                if(last_encoder_update_tick != 0) {
                    uart_tx_str("remained ticks for reset: "); uart_tx_uint32_text(3000 - (sys_tick - last_encoder_update_tick) ); uart_tx('\r'); uart_tx('\n');
                }
                uart_tx_str("couter: "); uart_tx('0' + (counter / 100)); uart_tx('0' + (counter % 100 / 10)); uart_tx('0' + (counter % 10)); uart_tx('\r'); uart_tx('\n');
                uart_tx_str("duty: "); uart_tx('0' + (buf / 100)); uart_tx('0' + (buf % 100 / 10)); uart_tx('0' + (buf % 10)); uart_tx('\r'); uart_tx('\n');
            }
            else if (g_rxByte == '\r') {
                uart_tx('\n');
            }
            else if (g_rxByte == '\n') {
                uart_tx('\r');
            }
            
            prev_Byte_2 = prev_Byte;
            prev_Byte = g_rxByte;
            g_rxByte = 0;
        }
    }
}
