#include "esp_zigbee_core.h"
#include "ha/esp_zigbee_ha_standard.h"
#include "esp_check.h"

#include "util/zigbee.h"
#include "clusters/temperature.h"
#include "clusters/light.h"

static const char *TAG = "Zigbee";

static void add_basic_cluster(esp_zb_cluster_list_t *cluster_list)
{
    esp_zb_light_sensor_cfg_t base_config = {
        .basic_cfg =
            {
                .zcl_version = ESP_ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE,
                .power_source = 0x03,
            },
        .identify_cfg =
            {
                .identify_time = ESP_ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE,
            },
    };

    esp_zb_attribute_list_t *basic_cluster = esp_zb_basic_cluster_create(&(base_config.basic_cfg));
    ESP_ERROR_CHECK(esp_zb_basic_cluster_add_attr(basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, MANUFACTURER_NAME));
    ESP_ERROR_CHECK(esp_zb_basic_cluster_add_attr(basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, MODEL_IDENTIFIER));
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_basic_cluster(cluster_list, basic_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));

    ESP_ERROR_CHECK(esp_zb_cluster_list_add_identify_cluster(cluster_list, esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY), ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE));
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_identify_cluster(cluster_list, esp_zb_identify_cluster_create(&(base_config.identify_cfg)), ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));

    ESP_LOGD(TAG, "Basic clusters added");
}

void set_endpoints()
{
    esp_zb_ep_list_t *endpoint_list = esp_zb_ep_list_create();
    esp_zb_endpoint_config_t endpoint_config = {
        .endpoint = 0x01,
        .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .app_device_id = ESP_ZB_HA_LIGHT_SENSOR_DEVICE_ID,
        .app_device_version = 0};

    esp_zb_cluster_list_t *cluster_list = esp_zb_zcl_cluster_list_create();

    // Basics
    add_basic_cluster(cluster_list);

    // Sensors
    light_sensor_register_cluster(cluster_list);
    temperature_sensor_register_cluster(cluster_list);

    // Add endpoint
    esp_zb_ep_list_add_ep(endpoint_list, cluster_list, endpoint_config);

    /* Register the device */
    ESP_ERROR_CHECK(esp_zb_device_register(endpoint_list));
}