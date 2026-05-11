#ifndef F_CPU
#define F_CPU       16000000
#endif

#ifndef BAUD
#define BAUD        38400
#endif

#ifndef __PIN_MAP_H__
#define __PIN_MAP_H__

#include <avr/io.h>

// PWM LED
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED_BIT     0

// encoder
#define ENC_CLK_DDR     INT0_DDR
#define ENC_CLK_PIN     INT0_PIN
#define ENC_CLK_PORT    INT0_PORT
#define ENC_CLK_BIT     INT0_BIT

#define ENC_DT_DDR      DDRA
#define ENC_DT_PIN      PINA
#define ENC_DT_PORT     PORTA
#define ENC_DT_BIT      1

#define ENC_SW_DDR      DDRD
#define ENC_SW_PIN      PIND
#define ENC_SW_PORT     PORTD
#define ENC_SW_BIT      6

// keyboard
#define KB_CLK_DDR      INT1_DDR
#define KB_CLK_PIN      INT1_PIN
#define KB_CLK_PORT     INT1_PORT
#define KB_CLK_BIT      INT1_BIT

#define KB_DT_DDR       DDRD
#define KB_DT_PIN       PIND
#define KB_DT_PORT      PORTD
#define KB_DT_BIT       4

// buzzer
#define BUZ_DDR     OC1A_DDR
#define BUZ_PORT    OC1A_PORT
#define BUZ_PIN     OC1A_PIN
#define BUZ_BIT     OC1A_BIT

// sh1106
#define OLED_CS_DDR     DDRB
#define OLED_CS_PORT    PORTB
#define OLED_CS_BIT     5

#define OLED_DC_DDR     DDRB
#define OLED_DC_PORT    PORTB
#define OLED_DC_BIT     1

#endif