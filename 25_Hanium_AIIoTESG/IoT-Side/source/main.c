#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "esp_log.h"

// App components
#include "app_config.h"
#include "blink.h"
#include "wifi_handler.h"
#include "sntp.h"
#include "mqtt_handler.h"
#include "dht22.h"
#include "adc.h"
#include "cds.h"
#include "wattMeter.h"

#include "relay.h"

static const char *TAG = "APP_MAIN";

static void main_data_aggregator_task(void *pvParameter);

void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // --- Initialize all modules ---
    ESP_LOGI(TAG, "Initializing modules...");
    ESP_ERROR_CHECK(blink_init(STAT_LED0_GPIO, STAT_LED1_GPIO));
    ESP_ERROR_CHECK(wifi_init_sta());
    
#if RELAY_ATTACHED
    ESP_ERROR_CHECK(toggle_init(RELAY_GPIO));
#endif
#if DHT_SENSOR_ATTACHED
    ESP_ERROR_CHECK(dht22_init(DHT22_GPIO));
#endif
#if CDS_SENSOR_ATTACHED
    ESP_ERROR_CHECK(cds_init(ADC_UNIT_1, CDS_CHANNEL, ADC_ATTEN_DB_12, ADC_BITWIDTH_12, "CDS_SENSOR"));
#endif
#if WATT_METER_ATTACHED
    ESP_ERROR_CHECK(watt_meter_init(ADC_UNIT_1, WATT_METER_CHANNEL, ADC_ATTEN_DB_12, ADC_BITWIDTH_12, "WATT_METER"));
#endif

    // --- Create tasks ---
    ESP_LOGI(TAG, "Creating tasks...");
    blink_create_task();
    wifi_create_wps_task(BUTTON_GPIO);

    // --- Wait for Wi-Fi connection ---
    ESP_LOGI(TAG, "Waiting for Wi-Fi connection...");
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    ESP_LOGI(TAG, "Wi-Fi Connected!");

    // --- Services that require network ---
    ESP_ERROR_CHECK(sntp_init_and_sync());
    ESP_ERROR_CHECK(mqtt_app_start());

    // Create the main data aggregator and publisher task
    xTaskCreate(main_data_aggregator_task, "main_data_task", 4096, NULL, MQTT_SEND_TASK_PRIORITY, NULL);

    ESP_LOGI(TAG, "Initialization complete. Main loop is running.");
}

static void main_data_aggregator_task(void *pvParameter) {
    // Give sensors some time to get their first reading
    vTaskDelay(pdMS_TO_TICKS(SENSOR_PERIOD_MS));

    const TickType_t interval = pdMS_TO_TICKS(PUBLISH_PERIOD_MS);
    TickType_t last_wake_time = xTaskGetTickCount();

    while (1) {
        sensor_data_t current_data = {0};

#if DHT_SENSOR_ATTACHED
        dht22_get_data(&current_data.temp, &current_data.humidity);
#else
        current_data.temp = -999;
        current_data.humidity = -999;
#endif

#if CDS_SENSOR_ATTACHED
        cds_sensor_read_lux(&current_data.brightness);
#else
        current_data.brightness = -999;
#endif

#if WATT_METER_ATTACHED
        // read_AC_current(&current_data.ampere);
        currentFourierAnalysis(&current_data.ampere);
#else
        current_data.ampere = -999; // Placeholder
#endif

#if RELAY_ATTACHED
        current_data.onoff = toggle_get_state();
#else
        current_data.onoff = false;
#endif
        
        // Publish data via MQTT
        mqtt_publish_data(&current_data);

        vTaskDelayUntil(&last_wake_time, interval);
    }
}
