#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include <time.h>
#include <sys/time.h>
#include "cJSON.h"

#include "app_config.h"
#include "mqtt_handler.h"
#include "blink.h"
#include "relay.h"

EventGroupHandle_t mqtt_event_group;
static const char *TAG = "MQTT_HANDLER";
static esp_mqtt_client_handle_t client;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "✅ MQTT Connected");
            xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);
            blink_set_error_state(ERROR_NONE); // All connected
            esp_mqtt_client_subscribe(client, MQTT_SUB_TOPIC, 0);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "❌ MQTT Disconnected");
            xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
            // Don't set WIFI_NOT_CONNECTED here, as Wi-Fi might still be up.
            // The wifi_handler will set the appropriate error state.
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
#if RELAY_ATTACHED
            // onoff control
            if (strncmp(event->topic, MQTT_SUB_TOPIC, event->topic_len) == 0) {
                printf("TOPIC=%.*s\n", event->topic_len, event->topic);
                printf("DATA=%.*s\n", event->data_len, event->data);
                cJSON *root = cJSON_ParseWithLength(event->data, event->data_len);
                if (root == NULL) {
                    ESP_LOGE(TAG, "Failed to parse JSON data");
                    return;
                }

                cJSON *deviceCode_json = cJSON_GetObjectItem(root, "deviceCode");
                if (!cJSON_IsString(deviceCode_json) || (deviceCode_json->valuestring == NULL) || (strcmp(deviceCode_json->valuestring, MQTT_DEVICE_CODE) != 0)) {
                    ESP_LOGE(TAG, "Invalid device code or device code mismatch");
                    cJSON_Delete(root);
                    return;
                }

                cJSON *command_json = cJSON_GetObjectItem(root, "command");
                if (cJSON_IsString(command_json) && (command_json->valuestring != NULL)) {
                    char *command = command_json->valuestring;
                    if (strcmp(command, "ON") == 0) {
                        toggle_set_state(true);
                    } else if (strcmp(command, "OFF") == 0) {
                        toggle_set_state(false);
                    } else if (strcmp(command, "GET") == 0) {
                        mqtt_send_relay_state(toggle_get_state());
                    }
                }
                cJSON_Delete(root);
            }
#endif
            break;
        default:
            break;
    }
}

esp_err_t mqtt_app_start(void) {
    mqtt_event_group = xEventGroupCreate();
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URL,
        .credentials.username = MQTT_PUB_TOPIC, // Using topic as username for this example
        .credentials.authentication.password = MQTT_PASSWORD,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    return esp_mqtt_client_start(client);
}

void mqtt_publish_data(const sensor_data_t *data) {
    if (!(xEventGroupGetBits(mqtt_event_group) & MQTT_CONNECTED_BIT)) {
        ESP_LOGE(TAG, "MQTT client not connected, cannot publish.");
#if IGNORE_MQTT_CONNECTED
#else
        return;
#endif
    }

    char payload[256];
    char time_buf[25];
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);

    snprintf(payload, sizeof(payload),
             "{\"deviceCode\": \"%s\", "
             "\"timestamp\": \"%s\", "
             "\"temp\": %.2f, "
             "\"humidity\": %.2f, "
             "\"brightness\": %d, "
             "\"electric\": %.1f,"
             "\"onoff\": \"%s\"}"
             , MQTT_DEVICE_CODE, time_buf, data->temp, data->humidity, data->brightness, data->ampere, ((data->onoff)?"ON":"OFF"));

    int msg_id = esp_mqtt_client_publish(client, MQTT_PUB_TOPIC, payload, 0, 1, 0);
    if (msg_id != -1) {
        ESP_LOGI(TAG, "Sent publish successful, msg_id=%d", msg_id);
        ESP_LOGI(TAG, "Payload: %s", payload);
    } else {
        ESP_LOGE(TAG, "Failed to publish message");
    }
}

void mqtt_send_relay_state(bool relay_state) {
    if (!(xEventGroupGetBits(mqtt_event_group) & MQTT_CONNECTED_BIT)) {
        ESP_LOGE(TAG, "MQTT client not connected, cannot publish.");
        return;
    }

    char payload[128];
    char time_buf[25];
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);

    snprintf(payload, sizeof(payload),
             "{\"deviceCode\": \"%s\", "
             "\"state\": \"%s\", "
             "\"timestamp\": \"%s\"}", 
             MQTT_DEVICE_CODE, relay_state ? "ON" : "OFF", time_buf);

    int msg_id = esp_mqtt_client_publish(client, MQTT_SUB_TOPIC, payload, 0, 1, 0);
    if (msg_id != -1) {
        ESP_LOGI(TAG, "Sent publish successful, msg_id=%d", msg_id);
        ESP_LOGI(TAG, "Payload: %s", payload);
    } else {
        ESP_LOGE(TAG, "Failed to publish message");
    }
}