#include "xpt2046.h"
#include "main.h"
#include <stdio.h> // For printf
#include <stdbool.h>
#include <stdint.h>

#define MEDIAN_SAMPLES 5

void xpt_init(point* pt, SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_port, uint16_t cs_pin) {
    pt->m_x = 0;
    pt->m_y = 0;
    pt->m_hspi = hspi;
    pt->m_Port = cs_port;
    pt->m_Pin = cs_pin;

    xpt_unselect(pt); // Ensure CS is high initially
}

static void _xpt_swap(uint16_t* arr, int i, int j) {
    uint16_t temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}

// 5개 배열을 6번의 swap으로 중간 값을 찾는 알고리즘(알고리즘 교재 발췌)
static uint16_t _xpt_find_median(uint16_t* a) {
    if (a[0] < a[1])
        _xpt_swap(a, 0, 1);
    if (a[2] < a[3])
        _xpt_swap(a, 2, 3);
    if (a[0] < a[2]) {
        _xpt_swap(a, 0, 2);
        _xpt_swap(a, 1, 3);
    }
    if (a[1] < a[4])
        _xpt_swap(a, 1, 4);

    if (a[1] > a[2]) {
        if (a[2] > a[4]) {
            return a[2];
        } else {
            return a[4];
        }
    } else {
        if (a[1] > a[3]) {
            return a[1];
        } else {
            return a[3];
        }
    }
}

// SPI 전송 및 수신
static bool _xpt_read_raw(point* pt, uint16_t* x, uint16_t* y) {
    uint8_t tx_buf[6] = { 0x90, 0x00, 0x00, 0xD0, 0x00, 0x00 };
    uint8_t rx_buf[6];

    xpt_select(pt);
    HAL_SPI_TransmitReceive(pt->m_hspi, tx_buf, rx_buf, 6, 10);
    xpt_unselect(pt);

    uint16_t raw_x = ((rx_buf[1] << 8) | rx_buf[2]) >> 3;
    uint16_t raw_y = ((rx_buf[4] << 8) | rx_buf[5]) >> 3;

    // Basic boundary check for noise
    if(raw_x < 300 || raw_x > 4000 || raw_y < 300 || raw_y > 4000) {
        return false;
    }

    *x = raw_x;
    *y = raw_y;
    return true;
}

// 5번 받아온 값을 통해 최종 값 생성
bool xpt_read(point* pt) {
    uint16_t x_samples[MEDIAN_SAMPLES];
    uint16_t y_samples[MEDIAN_SAMPLES];
    uint8_t samples_count = 0;
    uint8_t max_retries = MEDIAN_SAMPLES + 3; // Allow a few retries

    for (int i = 0; i < max_retries && samples_count < MEDIAN_SAMPLES; i++) {
        if (_xpt_read_raw(pt, &x_samples[samples_count], &y_samples[samples_count])) {
            samples_count++;
        }
    }

    // If we couldn't get enough valid samples, return false
    if (samples_count < MEDIAN_SAMPLES) {
        return false;
    }

    // medain 필터
    pt->m_x = _xpt_find_median(x_samples);
    pt->m_y = _xpt_find_median(y_samples);

    // Convert raw ADC values to screen coordinates
    // Map X
    if (pt->m_x < MIN_X) pt->m_x = MIN_X;
    if (pt->m_x > MAX_X) pt->m_x = MAX_X;
    pt->m_x = (uint16_t)(((float)(pt->m_x - MIN_X) * SCREEN_WIDTH) / (MAX_X - MIN_X));

    // Map Y
    if (pt->m_y < MIN_Y) pt->m_y = MIN_Y;
    if (pt->m_y > MAX_Y) pt->m_y = MAX_Y;
    pt->m_y = (uint16_t)(((float)(pt->m_y - MIN_Y) * SCREEN_HEIGHT) / (MAX_Y - MIN_Y));

    return true;
}