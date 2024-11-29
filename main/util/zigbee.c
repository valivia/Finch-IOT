#include "esp_zigbee_core.h"
#include "esp_check.h"

#include "main.h"

void add_basic_cluster(esp_zb_cluster_list_t *cluster_list, esp_zb_basic_cluster_cfg_t *basic_cfg)
{
    basic_cfg->power_source = 0x03;
    esp_zb_attribute_list_t *basic_cluster = esp_zb_basic_cluster_create(basic_cfg);
    ESP_ERROR_CHECK(esp_zb_basic_cluster_add_attr(basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, MANUFACTURER_NAME));
    ESP_ERROR_CHECK(esp_zb_basic_cluster_add_attr(basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, MODEL_IDENTIFIER));
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_basic_cluster(cluster_list, basic_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
}