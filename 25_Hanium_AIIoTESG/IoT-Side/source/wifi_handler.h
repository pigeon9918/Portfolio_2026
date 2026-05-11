#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include "esp_err.h"
#include "freertos/event_groups.h"

// Event group bit definitions
#define WIFI_CONNECTED_BIT BIT0

// Global event group handle for other modules to wait on
extern EventGroupHandle_t wifi_event_group;

// Function Prototypes
esp_err_t wifi_init_sta(void);
void wifi_create_wps_task(int button_gpio);

#endif // WIFI_HANDLER_H
