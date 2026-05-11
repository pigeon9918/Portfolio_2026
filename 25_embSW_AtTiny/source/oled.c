#include <avr/io.h>
#include "pinMap.h"

#include "oled.h"
#include "spi.h"
#include <avr/pgmspace.h>

void sh1106_set_location(uint8_t page, uint8_t column) {
    //// DC  0
    OLED_DC_PORT &=  ~(1 << OLED_DC_BIT);

    SpiUSITx(0b10110000 | (page & 0x0F));
    SpiUSITx(0b00000000 | (column & 0x0F));
    SpiUSITx(0b00010000 | ((column & 0xF0) >> 4) );
}

void sh1106_init() {
    const uint8_t init_commands[] = {
        0xae, 0x00, 0x10, 0x40, 0x81, 0x80, 0xC0, 0xa8,
        0x3f, 0xd3, 0x00, 0xd5, 0x50, 0xd9, 0x22, 0xda,
        0x12, 0xdb, 0x35, 0xa4, 0xa6, 0xaf
    };
    //// CS, DC 핀의 DDR 설정.
    OLED_CS_DDR |=   1 << OLED_CS_BIT;
    OLED_DC_DDR |=   1 << OLED_DC_BIT;
    //// CS  1 (초기값)
    OLED_CS_PORT |=  1 << OLED_CS_BIT;
    //// CS  0
    OLED_CS_PORT &=  ~(1 << OLED_CS_BIT);
    //// DC  0
    OLED_DC_PORT &=  ~(1 << OLED_DC_BIT);

    for (uint8_t i=0; i<sizeof(init_commands); i++) {
        SpiUSITx(init_commands[i]);
    }
    //// CS  1
    OLED_CS_PORT |= 1 << OLED_CS_BIT;
}

void sh1106_clear() {
    //// CS  0
    OLED_CS_PORT &=  ~(1 << OLED_CS_BIT);

    for (uint8_t page=0; page<8; page++) {
        sh1106_set_location(page, 0);
        //// DC  1
        OLED_DC_PORT |= (1 << OLED_DC_BIT);

        for (uint8_t count=0; count<132; count++) {
            SpiUSITx(0x00);
        }
    }
    //// CS  1
    OLED_CS_PORT |=  1 << OLED_CS_BIT;
}


void sh1106_testpattern() {
    //// CS  0
    OLED_CS_PORT &=  ~(1 << OLED_CS_BIT);

    for (uint8_t page=0; page<8; page++) {
        sh1106_set_location(page, 34);
        //// DC  1
        OLED_DC_PORT |= (1 << OLED_DC_BIT);
        
        for (uint8_t count=0; count<64; count++) {
            SpiUSITx(0xFF);
        }
    }
    //// CS  1
    OLED_CS_PORT |= 1 << OLED_CS_BIT;
}

void sh1106_border() {
    //// CS  0
    OLED_CS_PORT &=  ~(1 << OLED_CS_BIT);

    // Top Border
    sh1106_set_location(0, 0);

    //// DC  1
    OLED_DC_PORT |= (1 << OLED_DC_BIT);
    for (uint8_t column=0; column<132; column++) {
        SpiUSITx(0x01);
    }

    // Bottom Border
    sh1106_set_location(7, 0);
    //// DC  1
    OLED_DC_PORT |= (1 << OLED_DC_BIT);
    for (uint8_t column=0; column<132; column++) {
        SpiUSITx(0x80);
    }
    for (uint8_t page=0; page<8; page++) {
        // Left Border
        sh1106_set_location(page, 2);
        //// DC  1
        OLED_DC_PORT |=  (1 << OLED_DC_BIT);
        SpiUSITx(0xff);
        
        // Right Border
        sh1106_set_location(page, 129);
        
        //// DC  1
        OLED_DC_PORT |=  (1 << OLED_DC_BIT);
        SpiUSITx(0xff);
    }
    //// CS  1
    OLED_CS_PORT |=  1 << OLED_CS_BIT;
}

void sh1106_text_font24(const char * font_addr, uint8_t font_width, uint8_t page, uint8_t column) {
    unsigned l_fill_width = font_width<16 ? (16-font_width)>>1 : 0;
    unsigned r_fill_width = (font_width+l_fill_width)<16 ? 16-(font_width+l_fill_width) : 0;
    //// CS  0
    OLED_CS_PORT &= ~(1 << OLED_CS_BIT);
    for (unsigned y=0; y<24/8; y++) {
        sh1106_set_location(page+y, column);
        //// DC  1
        OLED_DC_PORT |= 1 << OLED_DC_BIT;
        for (unsigned x=0; x<l_fill_width; x++)
            SpiUSITx(0);                           
        // Font 왼쪽의 공백을 0으로 채움.
        for (unsigned x=0; x<font_width; x++)
            SpiUSITx(pgm_read_byte(font_addr++));  // Font 데이터를 Program Memory에서 가져와서 SPI로 전송.
        for (unsigned x=0; x<r_fill_width; x++)
            SpiUSITx(0);                           
        // Font 오른쪽의 공백을 0으로 채움.
    }
    //// CS  1
    OLED_CS_PORT |= 1 << OLED_CS_BIT;
}

void sh1106_graphic8(const char * image, uint8_t page, uint8_t column) {
    //// CS  0
    OLED_CS_PORT &= ~(1 << OLED_CS_BIT);
    sh1106_set_location(page, column);
    
    for (unsigned y=0; y<8; y++) {
        //// DC  1
        OLED_DC_PORT |= 1 << OLED_DC_BIT;
        SpiUSITx(pgm_read_byte(image++));
    }

    //// CS  1
    OLED_CS_PORT |= 1 << OLED_CS_BIT;
}

void sh1106_draw_tire() {
    //// CS  0
    OLED_CS_PORT &=  ~(1 << OLED_CS_BIT);

    // Left Border
    sh1106_set_location(1, 64);
    //// DC  1
    OLED_DC_PORT |= (1 << OLED_DC_BIT);
    SpiUSITx(0b00111100);
    
    // Right Border
    sh1106_set_location(1, 64 + 9);
    //// DC  1
    OLED_DC_PORT |= (1 << OLED_DC_BIT);
    SpiUSITx(0b00111100);
    
    // Top Border
    sh1106_set_location(0, 67);
    //// DC  1
    OLED_DC_PORT |=  (1 << OLED_DC_BIT);
    for(int i=0; i<4; i++)
        SpiUSITx(0b10000000);
        
    // Bottom Border
    sh1106_set_location(2, 67);
    //// DC  1
    OLED_DC_PORT |=  (1 << OLED_DC_BIT);
    for(int i=0; i<4; i++)
        SpiUSITx(0b00000001);

    //// CS  1
    OLED_CS_PORT |=  1 << OLED_CS_BIT;
}

void sh1106_draw_guage_border() {
    //// CS  0
    OLED_CS_PORT &=  ~(1 << OLED_CS_BIT);

    // Top Border
    sh1106_set_location(5, 0);

    //// DC  1
    OLED_DC_PORT |= (1 << OLED_DC_BIT);
    for (uint8_t column=0; column<132; column++) {
        SpiUSITx(0b00001100);
    }

    // Bottom Border
    sh1106_set_location(7, 0);
    //// DC  1
    OLED_DC_PORT |= (1 << OLED_DC_BIT);
    for (uint8_t column=0; column<132; column++) {
        SpiUSITx(0b00110000);
    }
    // Left Border
    sh1106_set_location(6, 2);
    //// DC  1
    OLED_DC_PORT |=  (1 << OLED_DC_BIT);
    SpiUSITx(0xff);
    SpiUSITx(0xff);
    
    // Right Border
    sh1106_set_location(6, 128);
    //// DC  1
    OLED_DC_PORT |=  (1 << OLED_DC_BIT);
    SpiUSITx(0xff);
    SpiUSITx(0xff);

    //// CS  1
    OLED_CS_PORT |=  1 << OLED_CS_BIT;
}

void sh1106_fill_guage(uint8_t current_spd) {
    //// CS  0
    OLED_CS_PORT &= ~(1 << OLED_CS_BIT);
    sh1106_set_location(6, 2);
    //// DC  1
    OLED_DC_PORT |=  (1 << OLED_DC_BIT);
    
    for(uint8_t i=0; i < current_spd; i++) {
        SpiUSITx(0xFF);
    }
    for(uint8_t i=current_spd; i < 128; i++) {
        SpiUSITx(0x00);
    }
    
    //// CS  1
    OLED_CS_PORT |=  1 << OLED_CS_BIT;
}

const uint8_t bulb_icon[] PROGMEM = {
    // Page 0 (Top)
    0x00, 0x00, 0x00, 0x80, 0xC0, (uint8_t)(0xC0 >> 1), (uint8_t)(0xC0 >> 2), (uint8_t)(0xC0 >> 3), (uint8_t)(0xC0 >> 4), (uint8_t)(0xC0 >> 5), (uint8_t)(0xC0 >> 5), (uint8_t)(0xC0 >> 6), (uint8_t)(0xC0 >> 6), (uint8_t)(0xC0 >> 6), (uint8_t)(0xC0 >> 6), (uint8_t)(0xC0 >> 6),
    (uint8_t)(0xC0 >> 6), (uint8_t)(0xC0 >> 6), (uint8_t)(0xC0 >> 6), (uint8_t)(0xC0 >> 6), (uint8_t)(0xC0 >> 6), (uint8_t)(0xC0 >> 5), (uint8_t)(0xC0 >> 5), (uint8_t)(0xC0 >> 4), (uint8_t)(0xC0 >> 3), (uint8_t)(0xC0 >> 2), (uint8_t)(0xC0 >> 1), 0xC0, 0x80, 0x00, 0x00, 0x00,
    // Page 1
    0x00, 0xFC, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFC, 0x00,
    // Page 2
    0x00, 0x01, 0x03, 0x07, 0x0E, 0x1C, 0x38, 0x30, 0x30, 0x30, 0x30, 0x30, 0xFF, 0xC1, 0xC2, 0xC1,
    0xC2, 0xC1, 0xC2, 0xFF, 0x30, 0x30, 0x30, 0x30, 0x30, 0x38, 0x1C, 0x0E, 0x07, 0x03, 0x01, 0x00,
    // Page 3 (Bottom)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x3C, 0xFF, 0xCC, 0xCC, 0xCC, 0xCC,
    0xCC, 0xCC, 0xCC, 0xCC, 0xFF, 0x3C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void sh1106_draw_bulb(uint8_t page, uint8_t column) {
    //// CS  0
    OLED_CS_PORT &= ~(1 << OLED_CS_BIT);
    
    for (uint8_t p = 0; p < 4; p++) {
        sh1106_set_location(page + p, column);
        //// DC  1
        OLED_DC_PORT |= (1 << OLED_DC_BIT);
        
        for (uint8_t i = 0; i < 32; i++) {
            SpiUSITx(pgm_read_byte(&bulb_icon[p * 32 + i]));
        }
    }

    //// CS  1
    OLED_CS_PORT |= 1 << OLED_CS_BIT;
}