#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wps.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "app_config.h"
#include "wifi_handler.h"
#include "blink.h"

EventGroupHandle_t wifi_event_group;
static const char *TAG = "WIFI_HANDLER";
static esp_wps_config_t wps_config = WPS_CONFIG_INIT_DEFAULT(WPS_TYPE_PBC);

// 32 + 1, 64 + 1 Bytes
static void save_wifi_creds(const char *ssid, const char *password) {
    nvs_handle_t nvs;
    ESP_ERROR_CHECK(nvs_open("wifi", NVS_READWRITE, &nvs));
    ESP_ERROR_CHECK(nvs_set_str(nvs, "ssid", ssid));
    ESP_ERROR_CHECK(nvs_set_str(nvs, "pass", password));
    ESP_ERROR_CHECK(nvs_commit(nvs));
    nvs_close(nvs);
    ESP_LOGI(TAG, "📦 Wi-Fi credentials saved to NVS");
    ESP_LOGI(TAG, "SSID: %s", ssid);
    ESP_LOGI(TAG, "Password: %s", password); // For debugging
}

// 32, 64 Bytes
static bool load_wifi_creds(char *ssid, char *password) {
    nvs_handle_t nvs;
    esp_err_t err = nvs_open("wifi", NVS_READONLY, &nvs);
    if (err != ESP_OK) {
        // This is not an error on first boot, so log as info
        ESP_LOGI(TAG, "NVS namespace 'wifi' not found or error opening: %s", esp_err_to_name(err));
        return false;
    }

    bool found = false; // Default to not found

    size_t ssid_len = 33;
    err = nvs_get_str(nvs, "ssid", ssid, &ssid_len);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "SSID found in NVS: %s", ssid);
        size_t pass_len = 65;
        err = nvs_get_str(nvs, "pass", password, &pass_len);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Password loaded successfully from NVS.");
            found = true; // Both loaded OK
        } else {
            ESP_LOGE(TAG, "Failed to get password from NVS. Error: %s", esp_err_to_name(err));
        }
    } else {
        ESP_LOGI(TAG, "Failed to get SSID from NVS. Error: %s", esp_err_to_name(err));
    }

    nvs_close(nvs);
    return found;
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
                // Do not connect here; connect only when credentials are loaded or after WPS
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "❌ Disconnected. Trying to reconnect...");
                xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
                blink_set_error_state(ERROR_WIFI_NOT_CONNECTED);
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_WPS_ER_SUCCESS:
                ESP_LOGI(TAG, "🔑 WPS Success");
                wifi_config_t wifi_config;
                esp_wifi_get_config(WIFI_IF_STA, &wifi_config);

                // Ensure credentials from WPS are null-terminated before saving
                char ssid[sizeof(wifi_config.sta.ssid) + 1] = {0};
                char password[sizeof(wifi_config.sta.password) + 1] = {0};

                // strncpy is safer as it handles non-null terminated source
                strncpy(ssid, (char*)wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid));
                strncpy(password, (char*)wifi_config.sta.password, sizeof(wifi_config.sta.password));

                save_wifi_creds(ssid, password);
                esp_wifi_wps_disable();
                // The device is already connected, no need to call esp_wifi_connect() again.
                // The IP_EVENT_STA_GOT_IP event will be triggered automatically.
                break;
            case WIFI_EVENT_STA_WPS_ER_FAILED:
            case WIFI_EVENT_STA_WPS_ER_TIMEOUT:
                ESP_LOGW(TAG, "WPS Failed or Timed Out");
                esp_wifi_wps_disable();
                // Revert to trying to connect with stored credentials
                esp_wifi_connect();
                break;
            default: break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "✅ Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        blink_set_error_state(ERROR_MQTT_NOT_CONNECTED); // Next state is MQTT
    }
}

esp_err_t wifi_init_sta(void) {
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    char ssid[33] = {0};
    char pass[65] = {0};
    wifi_config_t wifi_config = {0};

    bool credentials_found = load_wifi_creds(ssid, pass);

    if (credentials_found) {
        ESP_LOGI(TAG, "📂 Found stored credentials for SSID: %s", ssid);
        strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
        strncpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));
    } else {
        ESP_LOGI(TAG, "📭 No stored credentials. Start WPS by pressing the button.");
        // No credentials, will wait for WPS
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    if (credentials_found) {
        ESP_LOGI(TAG, "Connecting to Wi-Fi with stored credentials...");
        esp_wifi_connect();
    }

    return ESP_OK;
}

static void start_wps(void) {
    esp_wifi_disconnect();
    ESP_LOGI(TAG, "📲 Starting WPS...");
    blink_set_error_state(ERROR_WIFI_CONNECTING);
    esp_wifi_wps_disable(); // Ensure it's disabled before enabling
    ESP_ERROR_CHECK(esp_wifi_wps_enable(&wps_config));
    ESP_ERROR_CHECK(esp_wifi_wps_start(0));
}

static void wps_button_task(void *pvParameter) {
    int button_gpio = (int)pvParameter;
    gpio_config_t btn_config = {
        .pin_bit_mask = 1ULL << button_gpio,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&btn_config);

    bool last_state = gpio_get_level(button_gpio);
    while (1) {
        bool current_state = gpio_get_level(button_gpio);
        if (last_state == 1 && current_state == 0) { // Falling edge
            vTaskDelay(pdMS_TO_TICKS(20)); // Debounce
            if (gpio_get_level(button_gpio) == 0) {
                start_wps();
            }
        }
        last_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void wifi_create_wps_task(int button_gpio) {
    xTaskCreate(wps_button_task, "wps_button_task", 2048, (void*)button_gpio, WPS_TASK_PRIORITY, NULL);
}
