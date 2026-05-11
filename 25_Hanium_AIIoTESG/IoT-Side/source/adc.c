#include "adc.h"
#include <stdlib.h>
#include "esp_log.h"

static adc_oneshot_unit_handle_t adc1_handle = NULL;
static adc_oneshot_unit_handle_t adc2_handle = NULL;

adc_sensor_t* adc_sensor_create(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_bits_width_t width, const char* TAG) {
    adc_sensor_t* sensor = (adc_sensor_t*) malloc(sizeof(adc_sensor_t));
    if (sensor == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for sensor");
        return NULL;
    }
    sensor->unit = unit;
    sensor->channel = channel;
    sensor->atten = atten;
    sensor->width = width;
    sensor->TAG = TAG;
    sensor->cali_handle = NULL;
    
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = unit,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    adc_oneshot_chan_cfg_t adc_config = {
        .bitwidth = width,
        .atten = atten,
    };
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = unit,
        .atten = atten,
        .bitwidth = width,
    };

    adc_oneshot_unit_handle_t *unit_handle_ptr = NULL;
    if (unit == ADC_UNIT_1) {
        unit_handle_ptr = &adc1_handle;
    } else if (unit == ADC_UNIT_2) {
        unit_handle_ptr = &adc2_handle;
    }

    if (unit_handle_ptr && *unit_handle_ptr == NULL) {
        ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, unit_handle_ptr));
        ESP_LOGI(TAG, "ADC Unit %d initialized.", unit);
    } else if (unit_handle_ptr && *unit_handle_ptr != NULL) {
        ESP_LOGI(TAG, "ADC Unit %d already initialized.", unit);
    } else {
        ESP_LOGE(TAG, "Invalid ADC unit specified: %d", unit);
        free(sensor);
        return NULL;
    }

    ESP_ERROR_CHECK(adc_oneshot_config_channel(*unit_handle_ptr, channel, &adc_config));
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&(cali_config), &(sensor->cali_handle)));

    return sensor;
}

void adc_sensor_destroy(adc_sensor_t* sensor) {
    if (sensor != NULL) {
        // Note: adc_oneshot_del_unit and adc_cali_delete_scheme_line_fitting
        // should be called when the ADC unit is no longer needed,
        // typically when the application exits or the last sensor using it is destroyed.
        // For simplicity, we are not deleting the unit handle here.
        // Consider adding a reference counter if dynamic unit deletion is required.
        if (sensor->cali_handle) {
            adc_cali_delete_scheme_line_fitting(sensor->cali_handle);
        }
        free(sensor);
    }
}

int adc_sensor_read_raw(adc_sensor_t* sensor) {
    int ADC_read;
    adc_oneshot_unit_handle_t current_unit_handle = NULL;

    if (sensor->unit == ADC_UNIT_1) {
        current_unit_handle = adc1_handle;
    } else if (sensor->unit == ADC_UNIT_2) {
        current_unit_handle = adc2_handle;
    }

    if (current_unit_handle == NULL) {
        ESP_LOGE(sensor->TAG, "ADC unit handle is NULL for unit %d!", sensor->unit);
        return -1;
    }

    esp_err_t ret = adc_oneshot_read(current_unit_handle, sensor->channel, &ADC_read);
    if (ret != ESP_OK) {
        ESP_LOGE(sensor->TAG, "adc_oneshot_read failed: %s", esp_err_to_name(ret));
        return -1;
    }
    return ADC_read;
}

uint32_t adc_sensor_read_mv(adc_sensor_t* sensor) {
    int ADC_read = adc_sensor_read_raw(sensor);
    if (ADC_read == -1) {
        return 0; // Return 0 or handle error appropriately
    }
    int ADC_output;
    esp_err_t ret = adc_cali_raw_to_voltage(sensor->cali_handle, ADC_read, &ADC_output);
    if (ret != ESP_OK) {
        ESP_LOGE(sensor->TAG, "adc_cali_raw_to_voltage failed: %s", esp_err_to_name(ret));
        return 0; // Return 0 or handle error appropriately
    }
    return ADC_output;
}

/*
    // 센서 객체를 가리킬 포인터
    adc_sensor_t* cds_sensor = NULL;
    adc_sensor_t* power_sensor = NULL;

    // ... 기존 초기화 코드 ...
    // CDS 센서 객체 생성 (기존 cds_init() 대체)
    // GPIO 35는 ADC1_CHANNEL_7 입니다. 실제 핀에 맞게 수정하세요.
    cds_sensor = adc_sensor_create(ADC_UNIT_1, ADC1_CHANNEL_7, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, "CDS_SENSOR");
    // 전력 센서 객체 생성
    // GPIO 34는 ADC1_CHANNEL_6 입니다. 실제 핀에 맞게 수정하세요.
    power_sensor = adc_sensor_create(ADC_UNIT_1, ADC1_CHANNEL_6, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, "POWER_SENSOR");
    uint32_t cds_mv = adc_sensor_read_mv(cds_sensor);

    uint32_t power_mv = adc_sensor_read_mv(power_sensor);
    
    */