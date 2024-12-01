#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "esp_zigbee_core.h"

// Functions
void temperature_sensor_report();
void temperature_sensor_register_cluster(esp_zb_cluster_list_t *cluster_list);
void temperature_sensor_register_reporting_info();

#endif