#ifndef LIGHT_H
#define LIGHT_H

#include "esp_zigbee_core.h"

#define LIGHT_SENSOR_UPDATE_INTERVAL (5) /* Local sensor update interval (second) */
#define LIGHT_SENSOR_MIN_VALUE (0)       /* Local sensor min measured value (lux) */
#define LIGHT_SENSOR_MAX_VALUE (65535)   /* Local sensor max measured value (lux) */

#define LIGHT_SENSOR_ENDPOINT 11 /* esp temperature sensor device endpoint, used for temperature measurement */

// Functions
void light_sensor_report();
esp_err_t light_driver_init(void);
void light_sensor_register_ep(esp_zb_ep_list_t *endpoint_list);
void light_sensor_register_reporting_info();

#endif