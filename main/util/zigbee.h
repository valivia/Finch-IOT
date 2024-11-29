#ifndef ZIGBEE_H
#define ZIGBEE_H

#include "esp_zigbee_core.h"

void add_basic_cluster(esp_zb_cluster_list_t *cluster_list, esp_zb_basic_cluster_cfg_t *basic_cfg);

#endif