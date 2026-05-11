#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"

#include "relay.h"
static gpio_toggle_t* Relay = NULL;
static const char* TAG = "Relay";

esp_err_t toggle_init(int gpio_num) {
    Relay = malloc(sizeof(gpio_toggle_t));
    if (Relay == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for relay");
        return ESP_FAIL;
    }

    Relay->state = true;
    Relay->gpio_num = gpio_num;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio_num),
        .mode = GPIO_MODE_OUTPUT,
    };

    esp_err_t err = gpio_config(&io_conf);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Relay initialized on GPIO %d", gpio_num);
        gpio_set_level(gpio_num, Relay->state); // Set initial state
    } else {
        ESP_LOGE(TAG, "Relay initialization failed for GPIO %d", gpio_num);
        free(Relay);
        return ESP_FAIL;
    }
    return ESP_OK;
}

void toggle_set_state(bool state) {
    if (Relay != NULL) {
        Relay->state = state;
        gpio_set_level(Relay->gpio_num, Relay->state);
        ESP_LOGI(TAG, "Relay on GPIO %d turned %s", Relay->gpio_num, state ? "ON" : "OFF");
    }
}

void toggle_deinit() {
    if (Relay != NULL) {
        ESP_LOGI(TAG, "Deinitializing relay on GPIO %d", Relay->gpio_num);
        free(Relay);
    }
}

bool toggle_get_state() {
    return Relay->state;
}