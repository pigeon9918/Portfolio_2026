#ifndef BLINK_H
#define BLINK_H

#include <stdbool.h>
#include "esp_err.h"

// Enum for LED state control
typedef enum {
    LED_STATE_OFF,
    LED_STATE_ON,
    LED_STATE_INVERT
} led_state_t;

// Enum to identify LEDs
typedef enum {
    LED_0 = 0,
    LED_1 = 1,
    LED_ALL
} led_target_t;

// Enum for error states to control blinking patterns
typedef enum {
    ERROR_NONE,
    ERROR_WIFI_NOT_CONNECTED,
    ERROR_WIFI_CONNECTING,
    ERROR_MQTT_NOT_CONNECTED
} error_state_t;

// Function Prototypes
esp_err_t blink_init(int led0_gpio, int led1_gpio);
void blink_create_task(void);
void blink_set_state(led_target_t target, led_state_t state);
void blink_set_error_state(error_state_t error);

#endif // BLINK_H
