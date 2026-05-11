#ifndef CDS_H
#define CDS_H

#include "esp_err.h"
#include "adc.h"
#include "driver/adc.h"
#include "app_config.h"

esp_err_t cds_sensor_read_lux(int* data);
esp_err_t cds_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_bits_width_t width, const char* TAG);

#endif // CDS_H
