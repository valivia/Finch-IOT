#include "driver/temperature_sensor.h"

#define TEMP_SENSOR_UPDATE_INTERVAL (1) /* Local sensor update interval (second) */
#define TEMP_SENSOR_MIN_VALUE (-10)     /* Local sensor min measured value (degree Celsius) */
#define TEMP_SENSOR_MAX_VALUE (80)      /* Local sensor max measured value (degree Celsius) */

esp_err_t temperature_driver_init();
void get_temperature_readings(int16_t *temperature);