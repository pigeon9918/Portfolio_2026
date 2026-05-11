#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"

#include "app_config.h"
#include "dht22.h"

// Based on the original DHT22 driver by Ricardo Timmermann

#define DHT_OK 0
#define DHT_CHECKSUM_ERROR -1
#define DHT_TIMEOUT_ERROR -2

static const char* TAG = "DHT22";

static int dht_gpio;
static dht22_data_t sensor_data = {0.0f, 0.0f};
static SemaphoreHandle_t data_mutex;

// --- Internal function prototypes ---
static int read_dht_data();
static int get_signal_level(int us_timeout, bool state);

// --- Function Implementations ---

esp_err_t dht22_init(int gpio_num) {
    dht_gpio = gpio_num;
    gpio_set_pull_mode(gpio_num, GPIO_PULLUP_ONLY);
    data_mutex = xSemaphoreCreateMutex();
    if (data_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "DHT22 module initialized on GPIO %d", gpio_num);
    return ESP_OK;
}

esp_err_t dht22_get_data(float *temp, float *humidity) {
    read_dht_data();
    if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE) {
        *temp = sensor_data.temperature;
        *humidity = sensor_data.humidity;
        xSemaphoreGive(data_mutex);
        return ESP_OK;
    }
    return ESP_FAIL;
}

static int get_signal_level(int us_timeout, bool state) {
    int u_sec = 0;
    while(gpio_get_level(dht_gpio) == state) {
        if(u_sec > us_timeout) 
            return -1;
        ets_delay_us(1);
        u_sec++;
    }
    return u_sec;
}

static int read_dht_data() {
    uint8_t dht_data[5] = {0, 0, 0, 0, 0};

    gpio_set_direction(dht_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(dht_gpio, 0);
    ets_delay_us(1100); // 1.1ms
    gpio_set_level(dht_gpio, 1);
    ets_delay_us(30); // 30us
    gpio_set_direction(dht_gpio, GPIO_MODE_INPUT);

    if(get_signal_level(85, 0) == -1) return DHT_TIMEOUT_ERROR;
    if(get_signal_level(85, 1) == -1) return DHT_TIMEOUT_ERROR;

    for(int i = 0; i < 40; i++) {
        if(get_signal_level(56, 0) == -1) return DHT_TIMEOUT_ERROR;
        if(get_signal_level(75, 1) > 40) {
            dht_data[i/8] |= (1 << (7 - (i % 8)));
        }
    }

    if (dht_data[4] != ((dht_data[0] + dht_data[1] + dht_data[2] + dht_data[3]) & 0xFF)) {
        return DHT_CHECKSUM_ERROR;
    }

    if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE) {
        sensor_data.humidity = (float)(((dht_data[0] << 8) | dht_data[1])) / 10.0f;
        sensor_data.temperature = (float)(((dht_data[2] & 0x7F) << 8) | dht_data[3]) / 10.0f;
        if (dht_data[2] & 0x80) {
            sensor_data.temperature *= -1;
        }
        xSemaphoreGive(data_mutex);
        // ESP_LOGI(TAG, "Temp: %.1fC, Hum: %.1f%%", sensor_data.temperature, sensor_data.humidity);
    }

    return DHT_OK;
}
