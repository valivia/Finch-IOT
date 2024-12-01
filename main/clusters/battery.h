#ifndef BATTERY_H
#define BATTERY_H

#include "esp_zigbee_core.h"

// Functions
void battery_sensor_report();
void battery_sensor_register_cluster(esp_zb_cluster_list_t *cluster_list);
void battery_sensor_register_reporting_info();

#endif