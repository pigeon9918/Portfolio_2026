#ifndef DHT22_H
#define DHT22_H

#include "esp_err.h"

// Structure to hold sensor data
typedef struct {
    float temperature;
    float humidity;
} dht22_data_t;

// Function Prototypes
esp_err_t dht22_init(int gpio_num);
esp_err_t dht22_get_data(float *temp, float *humidity);

#endif // DHT22_H
