#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "app_config.h"
#include "blink.h"

#define NUM_OF_LEDS 2

static int led_gpios[NUM_OF_LEDS] = {-1, -1};
static bool led_states[NUM_OF_LEDS] = {false, false};
static error_state_t current_error_state = ERROR_WIFI_NOT_CONNECTED;
static const char *TAG = "BLINK";

static void blink_task(void *pvParameter) {
    TickType_t last_wake_time = xTaskGetTickCount();
    int to_wait_ms = 1000;

    while (1) {
        switch (current_error_state) {
            case ERROR_NONE:
                blink_set_state(LED_ALL, LED_STATE_OFF);
                to_wait_ms = 15000; // Long delay when no error
                break;
            case ERROR_WIFI_NOT_CONNECTED:
                blink_set_state(LED_0, LED_STATE_INVERT);
                blink_set_state(LED_1, LED_STATE_OFF);
                to_wait_ms = 1000;
                break;
            case ERROR_WIFI_CONNECTING:
                blink_set_state(LED_0, LED_STATE_INVERT);
                blink_set_state(LED_1, LED_STATE_OFF);
                to_wait_ms = 500;
                break;
            case ERROR_MQTT_NOT_CONNECTED:
                blink_set_state(LED_0, LED_STATE_OFF);
                blink_set_state(LED_1, LED_STATE_INVERT);
                to_wait_ms = 1000;
                break;
            default:
                blink_set_state(LED_ALL, LED_STATE_OFF);
                to_wait_ms = 1000;
                break;
        }
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(to_wait_ms));
    }
}

esp_err_t blink_init(int led0_gpio, int led1_gpio) {
    led_gpios[LED_0] = led0_gpio;
    led_gpios[LED_1] = led1_gpio;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << led_gpios[LED_0]) | (1ULL << led_gpios[LED_1]),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIOs");
        return err;
    }
    
    // Set initial state to OFF
    blink_set_state(LED_ALL, LED_STATE_OFF);
    ESP_LOGI(TAG, "Blink module initialized");
    return ESP_OK;
}

void blink_create_task(void) {
    xTaskCreate(blink_task, "blink_task", 2048, NULL, BLINK_TASK_PRIORITY, NULL);
}

void blink_set_state(led_target_t target, led_state_t state) {
    if (target == LED_ALL) {
        for (int i = 0; i < NUM_OF_LEDS; i++) {
            if (state == LED_STATE_INVERT) {
                led_states[i] = !led_states[i];
            } else {
                led_states[i] = (state == LED_STATE_ON);
            }
            gpio_set_level(led_gpios[i], led_states[i]);
        }
    } else if (target < NUM_OF_LEDS) {
        if (state == LED_STATE_INVERT) {
            led_states[target] = !led_states[target];
        } else {
            led_states[target] = (state == LED_STATE_ON);
        }
        gpio_set_level(led_gpios[target], led_states[target]);
    }
}

void blink_set_error_state(error_state_t error) {
    current_error_state = error;
}
