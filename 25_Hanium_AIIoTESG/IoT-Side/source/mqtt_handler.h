#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include "esp_err.h"
#include "freertos/event_groups.h"

// Event group bit definitions
#define MQTT_CONNECTED_BIT BIT0

// Structure for sensor data payload
typedef struct {
    float temp;
    float humidity;
    int brightness;
    float ampere;
    bool onoff;
} sensor_data_t;

// Global event group handle
extern EventGroupHandle_t mqtt_event_group;

// Function Prototypes
esp_err_t mqtt_app_start(void);
void mqtt_publish_data(const sensor_data_t *data);
void mqtt_send_relay_state(bool relay_state);

#endif // MQTT_HANDLER_H
