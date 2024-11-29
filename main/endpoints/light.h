#ifndef LIGHT_H
#define LIGHT_H

#include "esp_zigbee_core.h"

#define LIGHT_SENSOR_UPDATE_INTERVAL (5) /* Local sensor update interval (second) */
#define LIGHT_SENSOR_MIN_VALUE (0)       /* Local sensor min measured value (lux) */
#define LIGHT_SENSOR_MAX_VALUE (65535)   /* Local sensor max measured value (lux) */

// Functions
void light_sensor_report();
esp_err_t light_driver_init(void);
void light_sensor_register_cluster(esp_zb_cluster_list_t *cluster_list);
void light_sensor_register_reporting_info();

#endif