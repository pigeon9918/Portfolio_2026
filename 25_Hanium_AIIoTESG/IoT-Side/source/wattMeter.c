#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"

#include "app_config.h"
#include "wattMeter.h"

#include "esp_dsp.h"
#include <math.h>

static const char *TAG = "watt_meter";
static adc_sensor_t* watt_meter = NULL;
static SemaphoreHandle_t data_mutex;

#define N_SAMPLES 1024
int N = N_SAMPLES;
// Input test array
__attribute__((aligned(16)))
float x1[N_SAMPLES];
// Window coefficients
__attribute__((aligned(16)))
float wind[N_SAMPLES];
// working complex array, size must be N_SAMPLES * 2 for FFT
__attribute__((aligned(16)))
float y_cf[N_SAMPLES * 2];

esp_err_t watt_meter_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_bits_width_t width, const char* TAG) {
    watt_meter = adc_sensor_create(unit, channel, atten, width, TAG);
    data_mutex = xSemaphoreCreateMutex();
    if (data_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "watt meter initialized on ADC channel%d", channel);
    return ESP_OK;
}

// esp_err_t read_AC_current(float* current) {
//     int miliVoltage;             //analog value read from the sensor output pin
//     int maxValue = 0;            // store max value
//     int minValue = 3300;         // store min value

//     //sample for 0.2s
//     TickType_t start_time = xTaskGetTickCount();
//     while ((xTaskGetTickCount() - start_time) < pdMS_TO_TICKS(200)) {
//         miliVoltage = adc_sensor_read_mv(watt_meter);
//         if (miliVoltage > maxValue) {
//             maxValue = miliVoltage;
//         }
//         if (miliVoltage < minValue) {
//             minValue = miliVoltage;
//         }
//     }
//     double mVpp = (maxValue - minValue);
//     double Vrms = (mVpp / 2.0) * 0.707; //Vpp -> Vrms
//     double AmpPerV = 10; //for 16A
//     double ampere = Vrms / AmpPerV;
    
//     if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE) {
//         *current = ampere;
//         xSemaphoreGive(data_mutex);
//         return ESP_OK;
//     }
//     return ESP_FAIL;
// }

esp_err_t currentFourierAnalysis(float* current) {
    const int targetFreq = 60;
    const int SamplingFreq = 500;
    const int SamplingPeriod = 1000 / SamplingFreq;
    esp_err_t ret;
    ret = dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
    if (ret  != ESP_OK) {
        ESP_LOGE(TAG, "Not possible to initialize FFT. Error = %i", ret);
        return ESP_FAIL;
    }
    
    // --- band-Pass Filter Setup ---
    const float bpf_center_freq = (float)targetFreq; // Center frequency in Hz to remove DC and noise
    const float bpf_band_width = 30.0f;
    const float Q_fac = bpf_center_freq / bpf_band_width;
    __attribute__((aligned(16)))
    float biquad_coeffs[5];
    __attribute__((aligned(16)))
    float hpf_delay[2] = {0,0}; // Filter state

    // Generate band-pass filter coefficients
    dsps_biquad_gen_bpf_f32(biquad_coeffs, (bpf_center_freq / SamplingFreq), Q_fac);
    // --- End of HPF Setup ---

    // Sampling data from ADC and applying HPF
    TickType_t last_wake_time = xTaskGetTickCount();
    for (int i = 0; i < N_SAMPLES; i++) {
        float raw_sample = (float)adc_sensor_read_mv(watt_meter);
        // Apply HPF to the single raw sample and store it in x1
        dsps_biquad_f32(&raw_sample, &x1[i], 1, biquad_coeffs, hpf_delay);
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(SamplingPeriod));
    }

    // Apply Hann window (DC offset is already removed by HPF)
    dsps_wind_hann_f32(wind, N_SAMPLES);
    for (int i = 0; i < N_SAMPLES; i++) {
        y_cf[i * 2] = x1[i] * wind[i];
        y_cf[i * 2 + 1] = 0; // Imaginary part is zero
    }

    // FFT
    unsigned int start_b = dsp_get_cpu_cycle_count();
    dsps_fft2r_fc32(y_cf, N_SAMPLES);
    unsigned int end_b = dsp_get_cpu_cycle_count();
    // Bit reverse
    dsps_bit_rev_fc32(y_cf, N_SAMPLES);
    // Convert one complex vector to two complex vectors
    dsps_cplx2reC_fc32(y_cf, N_SAMPLES);

    // Calculate magnitude of each frequency bin
    for (int i = 0 ; i < N / 2 ; i++) {
        y_cf[i] = sqrt(y_cf[i * 2 + 0] * y_cf[i * 2 + 0] + y_cf[i * 2 + 1] * y_cf[i * 2 + 1]) / ((N_SAMPLES/2) * Q_fac);
    }
    int max_pos = 0;
    int max = 0;
    for(int i = 0; i < (N_SAMPLES / 2); i++){
        if(max < y_cf[i]) {
            max = y_cf[i];
            max_pos = i;
        }
    } 
    float mag_at_freq = 0;
    // Find the magnitude at the target frequency
    const int k = (int)((float)targetFreq * N_SAMPLES / SamplingFreq);
    if(abs(max_pos - k) < 2) {
        mag_at_freq = sqrt(y_cf[k]*y_cf[k] + y_cf[k+1]*y_cf[k+1]);
    }

    // Convert magnitude to RMS voltage, then to RMS current
    float Vrms = mag_at_freq / sqrt(2);
    float Irms = Vrms * 10.0; // Sensitivity: 10mA/mV for the sensor
    float Watt = Irms * 1.0 * 220 * 0.001; // assume that power factor is 0.9
    ESP_LOGI(TAG, "RMS Current at %dHz (bin %d): %.2f W", targetFreq, max_pos, Watt);

    // Show power spectrum in 64x20 window
    // ESP_LOGW(TAG, "Power Spectrum (Mag)");
    // dsps_view(y_cf, N_SAMPLES / 2, 64, 20, -10, 40, '|');
    ESP_LOGI(TAG, "FFT for %i complex points took %i cycles", N, end_b - start_b);

    if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE) {
        *current = Watt; // Store as integer
        xSemaphoreGive(data_mutex);
        return ESP_OK;
    }
    return ESP_FAIL;
}

// /*read DC Current Value*/
// static double readDCCurrent(int Pin)
// {
//     int analogValueArray[31];
//     for (int index = 0; index < 31; index++) {
//         analogValueArray[index] = analogRead(Pin);
//     }
//     int i, j, tempValue;
//     for (j = 0; j < 31 - 1; j ++) {
//         for (i = 0; i < 31 - 1 - j; i ++) {
//             if (analogValueArray[i] > analogValueArray[i + 1]) {
//                 tempValue = analogValueArray[i];
//                 analogValueArray[i] = analogValueArray[i + 1];
//                 analogValueArray[i + 1] = tempValue;
//             }
//         }
//     }
//     double medianValue = analogValueArray[(31 - 1) / 2];
//     double DCCurrentValue = (medianValue / ADC_RESOLUTION * Vref - Vref / 2.0) / mVperAmp;  //Sensitivity:100mV/A, 0A @ Vcc/2
//     return DCCurrentValue;
// }