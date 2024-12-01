#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "battery_adc.h"

static const char *TAG = "driver_battery";

// State
adc_oneshot_unit_handle_t adc_handle;
adc_cali_handle_t adc_cali_handle = NULL;

// Readings
static int raw_voltage = 0;
static int calibrated_voltage = 0;

// Outputs
static uint16_t voltage = 0;
static double percentage = 0;

static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

    ESP_LOGD(TAG, "calibration scheme version is %s", "Curve Fitting");
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = unit,
        .chan = channel,
        .atten = atten,
        .bitwidth = ADC_BITWIDTH,
    };
    ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
    if (ret == ESP_OK)
    {
        calibrated = true;
    }

    *out_handle = handle;
    if (ret == ESP_OK)
    {
        ESP_LOGD(TAG, "Calibration Success");
    }
    else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated)
    {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    }
    else
    {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

static void initialize_adc()
{
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
    };

    // 2. Initialize the ADC unit
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_handle));

    // 3. Configure the ADC channel
    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH, // 12-bit resolution
        .atten = ADC_ATTEN        // 11dB attenuation for 0-3.6V range
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL, &chan_cfg));
    adc_calibration_init(ADC_UNIT_1, ADC_CHANNEL, ADC_ATTEN, &adc_cali_handle);

    if (adc_cali_handle == NULL)
    {
        ESP_LOGE(TAG, "ADC calibration failed");
        return;
    }
}

static void read_voltage()
{
    gpio_set_direction(BATTERY_READ_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BATTERY_READ_PIN, 1);

    // Take a one-shot reading
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL, &raw_voltage));
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, raw_voltage, &calibrated_voltage));

    ESP_LOGD(TAG, "ADC Raw: %d, Calibrated: %d", raw_voltage, calibrated_voltage);

    gpio_set_level(BATTERY_READ_PIN, 0);
}

static double calculate_battery_percentage(int voltage)
{
    if (voltage > BATTERY_VOLTAGE_MAX_MILLIVOLT)
    {
        return 100.0;
    }
    else if (voltage < BATTERY_VOLTAGE_MIN_MILLIVOLT)
    {
        return 0.0;
    }

    return ((voltage - BATTERY_VOLTAGE_MIN_MILLIVOLT) / (BATTERY_VOLTAGE_MAX_MILLIVOLT - BATTERY_VOLTAGE_MIN_MILLIVOLT)) * 100.0;
}

esp_err_t battery_driver_init()
{
    initialize_adc();
    read_voltage();

    voltage = calibrated_voltage * 4;
    percentage = calculate_battery_percentage(voltage);

    ESP_LOGI(TAG, "Initialized, percentage: %f, mv: %d, raw: %d", percentage, voltage, calibrated_voltage);

    return ESP_OK;
}

void get_battery_readings(uint16_t *battery_voltage, double *battery_percentage)
{
    *battery_voltage = voltage;
    *battery_percentage = percentage;
}