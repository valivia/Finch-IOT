#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "esp_zigbee_core.h"

#define TEMP_SENSOR_UPDATE_INTERVAL (1) /* Local sensor update interval (second) */
#define TEMP_SENSOR_MIN_VALUE (-10)     /* Local sensor min measured value (degree Celsius) */
#define TEMP_SENSOR_MAX_VALUE (80)      /* Local sensor max measured value (degree Celsius) */

#define TEMPERATURE_SENSOR_ENDPOINT 10 /* esp temperature sensor device endpoint, used for temperature measurement */

// Functions
void temperature_sensor_report();
esp_err_t temperature_driver_init(void);
void temperature_sensor_register_ep(esp_zb_ep_list_t *endpoint_list);
void temperature_sensor_register_reporting_info();

#endif