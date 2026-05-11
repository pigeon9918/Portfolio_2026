#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"

#include "app_config.h"
#include "cds.h"

// static const char *TAG = "CDS_SENSOR";
static adc_sensor_t* cds_sensor = NULL;
static SemaphoreHandle_t data_mutex;

esp_err_t cds_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_bits_width_t width, const char* TAG) {
    cds_sensor = adc_sensor_create(unit, channel, atten, width, TAG);
    data_mutex = xSemaphoreCreateMutex();
    ESP_LOGI(TAG, "Attempting to create data_mutex...");
    if (data_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "CDS module initialized on ADC channel%d", channel);
    return ESP_OK;
}

esp_err_t cds_sensor_read_lux(int* brightness) {
    if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE) {
        int sum  = 0;
        for(int i = 0; i<5; i++) {
            int raw = adc_sensor_read_mv(cds_sensor);
            sum += raw;
        }
        int avg = sum / 3.0;
        int lux = 2.95 + 0.0311 * avg + 0.0000355 * avg * avg;
        // 2.95 + 0.0311x + 3.55E-05x^2
        // printf("first : %dlux\n", lux);
        *brightness = lux;
        xSemaphoreGive(data_mutex);
        return ESP_OK;
    }
    return ESP_FAIL;
}