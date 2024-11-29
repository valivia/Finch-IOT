#ifndef BATTERY_H
#define BATTERY_H

#include "esp_zigbee_core.h"

// Functions
void light_sensor_report();
esp_err_t battery_driver_init(void);
void battery_sensor_register_cluster(esp_zb_cluster_list_t *cluster_list);
void battery_sensor_register_reporting_info();

#endif