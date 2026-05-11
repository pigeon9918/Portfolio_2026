#include "main.h"
#include "ili9341.h"

#define ILI9341_SPI hspi1
extern SPI_HandleTypeDef ILI9341_SPI;

static inline void ILI9341_Select(void) {
    // ILI9341_CS  0
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, 0);
}

static inline void ILI9341_Unselect(void) {
    // ILI9341_CS  1
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, 1);
}

static inline void ILI9341_WriteCommand(uint8_t cmd) {
    // ILI9341_DC  0
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, 0);
    // hspi1 으로 1-byte의 cmd를 보냄.
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 500);
}

static void ILI9341_WriteData(uint8_t * buf, size_t buf_size) {
    // ILI9341_DC  1
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, 1);
    // HAL SPI 함수는 한번에 65535 bytes 까지만 보낼 수 있음. 65536 bytes 보다 크면 잘게 나눠서 보냄.
    while (buf_size > 0) {
        uint16_t chunk_size;

        // buf_size가 65535보다 크면 chunk_size  65535, 아니면 chunk_size  buf_size
        chunk_size = (buf_size > 65535) ? 65535 : buf_size;
        // spi1으로 chunk_size의 buf[]를 보냄.
        HAL_SPI_Transmit(&hspi1, buf, chunk_size, 500);

        buf += chunk_size;
        buf_size -= chunk_size;
    }
}

void ILI9341_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t buf[] = {
        (uint8_t)(x0 >> 8),
        (uint8_t)(x0 & 0x00FF),
        (uint8_t)(x1 >> 8),
        (uint8_t)(x1 & 0x00FF),
        (uint8_t)(y0 >> 8),
        (uint8_t)(y0 & 0x00FF),
        (uint8_t)(y1 >> 8),
        (uint8_t)(y1 & 0x00FF),
    };
    // 이미 ILI9341_Select()가 실행되었다고 가정.
    // Column Address Set (command 와 data 전송)
    ILI9341_WriteCommand(0x2A);
    ILI9341_WriteData(&buf[0], 4);
    // Row Address Set (command 와 data 전송)
    ILI9341_WriteCommand(0x2B);
    ILI9341_WriteData(&buf[4], 4);
    // Memory Write (command만 전송)
    ILI9341_WriteCommand(0x2C);
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    uint8_t data[2];
    if ((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT))
    return;
    ILI9341_Select();
    ILI9341_SetAddressWindow(x, y, x, y);
    data[0] = (uint8_t)(color >> 8);  
    // color의 상위 8 bits
    data[1] = (uint8_t)(color & 0x00FF);  
    // color의 하위 8 bits
    ILI9341_WriteData(data, sizeof(data));
    ILI9341_Unselect();
}

void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    static uint8_t buf[ILI9341_WIDTH*2];

    if ((x > ILI9341_WIDTH) || (y > ILI9341_HEIGHT))
        return;
    if ((x+w) > ILI9341_WIDTH)
        w = ILI9341_WIDTH-x;
    if ((y+h) > ILI9341_HEIGHT)
        h = ILI9341_HEIGHT-y;

    ILI9341_Select();
    ILI9341_SetAddressWindow(x, y, x + w -1, y + h -1);
    // buf[] 에 color 값을 채워 넣음
    for(int i = 0; i < w; i++) {
        buf[i * 2]     = (uint8_t)(color >> 8);   // MSB
        buf[i * 2 + 1] = (uint8_t)(color & 0xFF); // LSB
    }
    for(int i=0; i< h; i++) {
        // ILI9341_WriteData()을 사용하여 buf[] 를 전송.
        ILI9341_WriteData(buf, w * 2);
    }
    ILI9341_Unselect();
}

inline void ILI9341_FillScreen(uint16_t color) {
    ILI9341_FillRectangle(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, color);
}

void ILI9341_InvertColors(unsigned invert)
{
    uint8_t datas[] = {
        0x20,
        0x21,
    };
    ILI9341_Select();
    if(invert)
        ILI9341_WriteCommand(datas[1]);
    else
        ILI9341_WriteCommand(datas[0]);

    ILI9341_Unselect();
}

void ILI9341_Init(void) {
    ILI9341_Select();
    uint8_t datas[] = {
        0x05,
        ILI9341_ROTATION,
    };
    // COLMOD를 16 bits/pixel로 설정.
    ILI9341_WriteCommand(0x3A);
    ILI9341_WriteData(&datas[0], 1);
    
    // Memory Access Control을 rotate right로 설정.
    ILI9341_WriteCommand(0x36);
    ILI9341_WriteData(&datas[1], 1);

    // POSITIVE GAMMA CORRECTION
    ILI9341_WriteCommand(0xE0);
    { 
        uint8_t data[] = { 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
        0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // NEGATIVE GAMMA CORRECTION
    ILI9341_WriteCommand(0xE1);
    {
        uint8_t data[] = { 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
        0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F };
        ILI9341_WriteData(data, sizeof(data));
    }

    // Sleep Out command 전송
    ILI9341_WriteCommand(0x11);

    // 60ms 동안 delay
    HAL_Delay(60);
    
    // Display ON command 전송
    ILI9341_WriteCommand(0x29);

    ILI9341_Unselect();
}

void ILI9341_Test(void) {
    // Clear Screen
    ILI9341_FillScreen(ILI9341_RED);
    HAL_Delay(1000);
    ILI9341_FillScreen(ILI9341_GREEN);
    HAL_Delay(1000);
    ILI9341_FillScreen(ILI9341_BLUE);
    HAL_Delay(1000);
    ILI9341_FillScreen(ILI9341_BLACK);
    HAL_Delay(1000);
    // Draw Borders
    ILI9341_FillRectangle(0, 0, ILI9341_WIDTH, 1, ILI9341_RED);
    ILI9341_FillRectangle(0, ILI9341_HEIGHT-1, ILI9341_WIDTH, 1, ILI9341_RED);
    for (unsigned y = 0; y<ILI9341_HEIGHT; y++) {
        ILI9341_DrawPixel(0, y, ILI9341_RED);
        ILI9341_DrawPixel(ILI9341_WIDTH-1, y, ILI9341_RED);
    }
    HAL_Delay(1000);
    // Draw fonts
    ILI9341_FillScreen(ILI9341_BLACK);
    // ILI9341_WriteString(0, 20, "Font_7x10, Red, Test String 0123456789", Font_7x10, ILI9341_RED, ILI9341_BLACK);
    // ILI9341_WriteString(0, 20 + 3*10, "Font_11x18, Green, Test String 0123456789", Font_11x18, ILI9341_GREEN, ILI9341_BLACK);
    // ILI9341_WriteString(0, 20 + 3*10+3*18, "Font_16x26, Blue, Test String 0123456789", Font_16x26, ILI9341_BLUE, ILI9341_BLACK);
    HAL_Delay(1000);
    // Invert Colors
    ILI9341_InvertColors(1);
    HAL_Delay(1000);
    // Normal Colors
    ILI9341_InvertColors(0);
    HAL_Delay(1000);
}
