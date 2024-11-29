#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "driver/adc.h"
#include "driver/gpio.h"

#include "battery_adc.h"

static const char *TAG = "Battery ADC";

static void initialize_adc()
{
    adc1_config_width(ADC_WIDTH_BIT_12);                    // 12-bit ADC resolution
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_0); // 0 dB attenuation
}

static float read_battery_voltage()
{
    initialize_adc();

    int adc_value = adc1_get_raw(ADC_CHANNEL);
    ESP_LOGI(TAG, "ADC value: %d", adc_value);
    float voltage_at_io6 = (adc_value / ADC_MAX_VALUE) * ADC_REFERENCE_VOLTAGE;

    // Calculate the actual battery voltage using the voltage divider formula
    float battery_voltage = voltage_at_io6 / (VOLTAGE_DIVIDER_R2 / (VOLTAGE_DIVIDER_R1 + VOLTAGE_DIVIDER_R2));

    return battery_voltage;
}

static float calculate_battery_percentage(float battery_voltage)
{
    if (battery_voltage > BATTERY_VOLTAGE_MAX)
    {
        return 100.0;
    }
    else if (battery_voltage < BATTERY_VOLTAGE_MIN)
    {
        return 0.0;
    }

    return ((battery_voltage - BATTERY_VOLTAGE_MIN) / (BATTERY_VOLTAGE_MAX - BATTERY_VOLTAGE_MIN)) * 100.0;
}

void get_battery_percentage(float *battery_voltage, float *battery_percentage)
{
    *battery_voltage = read_battery_voltage();
    *battery_percentage = calculate_battery_percentage(*battery_voltage);
}