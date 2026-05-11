#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// including stdlib
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
// inlcuding esplib
#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "esp_netif.h"
#include "assert.h"

#define IGNORE_MQTT_CONNECTED   false

// --- SENSOR ATTACHMENT FLAGS ---
#define DHT_SENSOR_ATTACHED	    true
#define CDS_SENSOR_ATTACHED	    true
#define WATT_METER_ATTACHED	    false
#define RELAY_ATTACHED          false

// --- HARDWARE PINS ---
#define BUTTON_GPIO         GPIO_NUM_21
#define STAT_LED0_GPIO      GPIO_NUM_22
#define STAT_LED1_GPIO      GPIO_NUM_23
#define DHT22_GPIO          GPIO_NUM_33
#define RELAY_GPIO          GPIO_NUM_17
#define CDS_CHANNEL         ADC_CHANNEL_4
#define WATT_METER_CHANNEL  ADC_CHANNEL_7

// --- WIFI & MQTT ---
// #define MQTT_BROKER_URL     "mqtt://hanium-server.localdomain:1883"
#define MQTT_BROKER_URL		"mqtt://192.168.137.102:1883"
#define MQTT_DEVICE_CODE    "1234556"
#define MQTT_PUB_TOPIC      "power/measurement"
#define MQTT_SUB_TOPIC      "device/command" // Example subscription topic
#define MQTT_PASSWORD       "your_password" // Example password, consider more secure methods

// --- TASK CONFIGURATION ---
#define SENSOR_PERIOD_MS	    2000
#define PUBLISH_PERIOD_MS	    2000

#define BLINK_TASK_PRIORITY     3
#define WPS_TASK_PRIORITY       6
#define MQTT_SEND_TASK_PRIORITY 7
#define CDS_TASK_PRIORITY       8
#define DHT_TASK_PRIORITY       9

#endif // APP_CONFIG_H
