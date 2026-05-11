#ifndef __ADC_H__
#define __ADC_H__

#include "driver/adc.h"
#include "hal/adc_types.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

// ADC 센서의 모든 정보를 담는 구조체 (C++의 'class' 역할)
typedef struct {
    adc_unit_t unit;
    adc_channel_t channel;
    adc_atten_t atten;
    adc_bits_width_t width;
    
    adc_cali_handle_t cali_handle;  // 전압 계산을 위한 특성 값
    const char* TAG;                // 로그 출력을 위한 태그
} adc_sensor_t;

/**
 * @brief ADC 센서 객체를 생성하고 초기화합니다. (C++의 'constructor' 역할)
 * 
 * @param unit ADC 유닛 (ADC_UNIT_1 또는 ADC_UNIT_2)
 * @param channel ADC 채널
 * @param atten 감쇠 설정
 * @param width 비트 해상도
 * @param tag 로그용 태그
 * @return 생성된 adc_sensor_t 객체의 포인터. 실패 시 NULL.
 */
adc_sensor_t* adc_sensor_create(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_bits_width_t width, const char* tag);

/**
 * @brief ADC 센서 객체를 메모리에서 해제합니다. (C++의 'destructor' 역할)
 * 
 * @param sensor 해제할 센서 객체의 포인터
 */
void adc_sensor_destroy(adc_sensor_t* sensor);

/**
 * @brief 센서의 raw ADC 값을 읽습니다.
 * 
 * @param sensor 읽을 센서 객체
 * @return int Raw ADC 값
 */
int adc_sensor_read_raw(adc_sensor_t* sensor);

/**
 * @brief 센서의 전압(mV)을 읽습니다.
 * 
 * @param sensor 읽을 센서 객체
 * @return uint32_t 전압 (mV)
 */
uint32_t adc_sensor_read_mv(adc_sensor_t* sensor);

#endif // __ADC_H__
