#ifndef __OLED_H__
#define __OLED_H__

#include <avr/io.h>
#include "pinMap.h"

// OLED
void sh1106_init();
void sh1106_clear();
void sh1106_testpattern();
void sh1106_border();
void sh1106_set_location(uint8_t page, uint8_t column);
void sh1106_text_font24(const char * font_addr, uint8_t font_width, uint8_t page, uint8_t column);
void sh1106_graphic8(const char * image, uint8_t page, uint8_t column);
void sh1106_draw_guage_border();
void sh1106_fill_guage(uint8_t current_spd);
void sh1106_draw_tire();
void sh1106_draw_bulb(uint8_t page, uint8_t column);

#endif