#include "pinMap.h"
#include "keyboard.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

void INT1_KB_init() {
    //// Clock과 Data핀을 입력으로 설정.
    KB_CLK_DDR &= ~(1 << KB_CLK_BIT);
    KB_DT_DDR &= ~(1 << KB_DT_BIT);
    //// Clock과 Data핀의 pull-up 저항을 활성화.
    KB_CLK_PORT |= (1 << KB_CLK_BIT);
    KB_DT_PORT |= (1 << KB_DT_BIT);
    //// INT1를 falling edge에서 발생하도록 MCUCR 설정.
    MCUCR &= ~(0b11 << ISC10);
    MCUCR |= (0b10 << ISC10);
    //// INT0를 활성화 하도록 GIMSK 설정.
    GIMSK |= (1 << INT1);
}

ISR(INT1_vect) {
    static uint16_t receivedBits = 0;
    static uint8_t bitCount = 0;
    //// Data 핀을 읽어 1이면, receivedBits의 해당 bit를 1로 설정.
    uint8_t dat = (KB_DT_PIN >> KB_DT_BIT) & 1;
    receivedBits |= ((uint16_t)dat << bitCount);

    //// bitCount를 증가하고, 11이면
    bitCount++;
    if(bitCount == 11) {
        ////   receivedBits를 scanCode에 저장.
        scanCode = receivedBits;
        ////   receivedBits와 bitCount를 0으로 초기화.
        receivedBits = 0;
        bitCount = 0;
    }
}

uint8_t ps2_scan_to_ascii(uint16_t code) {
	uint8_t s, c;
	static uint8_t sBreak=0, sModifier=0, sShift=0;
	static const char keymap_unshifted[] PROGMEM =
		"             \011`      q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09"
		"  ./l;p-   \' [=    \015] \\        \010  1 47   0.2568\033  +3-*9      ";
	static const char keymap_shifted[] PROGMEM =
		"             \011~      Q!   ZSAW@  CXDE$#   VFTR%  NBHGY^   MJU&*  <KIO)("
		"  >?L:P_   \" {+    \015} |        \010  1 47   0.2568\033  +3-*9       ";

	s = (code>>1) & 0xff;		// Remove start, parity, stop bits

	if (s==0xaa)
		return 0;				// Ignore BAT completion code
	if (s==0xf0) {
		sBreak=1;
		return 0;
	}
	if (s==0xe0) {
		sModifier=1;
		return 0;
	}
	if (sBreak) {						// if key released
		if ((s==0x12) || (s==0x59)) {	// Left or Right Shift Key
			sShift=0;
		}
		sBreak=0; sModifier=0;
		return 0;
	}
	if ((s==0x12) || (s==0x59))			// Left or Right Shift Key
		sShift=1;
	if (sModifier)						// If modifier ON, return 0
		return 0;
	if (sShift==0)
		c=pgm_read_byte(keymap_unshifted+s);
	else
		c=pgm_read_byte(keymap_shifted+s);
	if ((c==32) && (s!=0x29))			// Ignore unless real space key
		return 0;
	return c;	
}