#ifndef __WATT_METER_H__
#define __WATT_METER_H__

#include "esp_err.h"
#include "adc.h"
#include "driver/adc.h"
#include "app_config.h"

esp_err_t watt_meter_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_bits_width_t width, const char* TAG);
esp_err_t read_AC_current(float* current);
esp_err_t currentFourierAnalysis(float* current);

#endif