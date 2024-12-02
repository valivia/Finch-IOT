#include "temp.h"

#include "esp_err.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "driver_temp";

static temperature_sensor_handle_t temp_sensor;
int16_t temperature_value = 0;

void read_temperature(int16_t *value)
{
    float temp;
    temperature_sensor_get_celsius(temp_sensor, &temp);
    *value = (int16_t)(temp * 100);
}

esp_err_t temperature_driver_init()
{
    temperature_sensor_config_t config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(TEMP_SENSOR_MIN_VALUE, TEMP_SENSOR_MAX_VALUE);

    // Install
    ESP_RETURN_ON_ERROR(
        temperature_sensor_install(&config, &temp_sensor),
        TAG,
        "Fail to install on-chip temperature sensor");

    // Enable
    ESP_RETURN_ON_ERROR(
        temperature_sensor_enable(temp_sensor),
        TAG,
        "Fail to enable on-chip temperature sensor");

    // Read
    read_temperature(&temperature_value);

    // Disable
    ESP_RETURN_ON_ERROR(
        temperature_sensor_disable(temp_sensor),
        TAG,
        "Fail to disable on-chip temperature sensor");

    ESP_LOGI(TAG, "Initialized, Temperature: %d", temperature_value);

    return ESP_OK;
}

void get_temperature_readings(int16_t *temperature)
{
    *temperature = temperature_value;
}
