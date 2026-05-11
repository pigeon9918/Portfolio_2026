#ifndef __XPT2046_H__
#define __XPT2046_H__

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#define MIN_X 300
#define MAX_X 3800
#define MIN_Y 300
#define MAX_Y 3800

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

typedef struct {
    uint16_t m_x;
    uint16_t m_y;
    SPI_HandleTypeDef* m_hspi;
    GPIO_TypeDef* m_Port;
    uint16_t m_Pin;
} point;

void xpt_init(point*, SPI_HandleTypeDef*, GPIO_TypeDef* cs_port, uint16_t cs_pin);
bool xpt_read(point*);

static inline void xpt_select(point* pt) {
    HAL_GPIO_WritePin(pt->m_Port, pt->m_Pin, 0);
}

static inline void xpt_unselect(point* pt) {
    HAL_GPIO_WritePin(pt->m_Port, pt->m_Pin, 1);
}

#endif