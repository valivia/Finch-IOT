#include "esp_zigbee_core.h"
#include "esp_log.h"
#include "esp_check.h"
#include "ha/esp_zigbee_ha_standard.h"
#include "zcl/esp_zigbee_zcl_power_config.h"

#include "battery.h"
#include "driver/battery_adc.h"
#include "util/zigbee.h"
#include "main.h"

static const char *TAG = "illuminance_zb";

float battery_percentage = 0;
float battery_voltage = 0;

/* Update sensor measured values */
static void battery_sensor_save()
{
    uint8_t battery_percentage_attr = (uint8_t)battery_percentage;
    uint8_t battery_voltage_attr = (uint8_t)battery_voltage;

    esp_zb_lock_acquire(portMAX_DELAY);
    esp_zb_zcl_set_attribute_val(ZB_ENDPOINT,
                                 ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                                 ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID, &battery_percentage_attr, false);
    esp_zb_zcl_set_attribute_val(ZB_ENDPOINT,
                                 ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                                 ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_ID, &battery_voltage_attr, false);
    esp_zb_lock_release();
}

void battery_sensor_report()
{
    esp_zb_zcl_report_attr_cmd_t report_attr_cmd = {0};
    report_attr_cmd.address_mode = ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT;
    report_attr_cmd.attributeID = ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID;
    report_attr_cmd.direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_CLI;
    report_attr_cmd.clusterID = ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG;
    report_attr_cmd.zcl_basic_cmd.src_endpoint = ZB_ENDPOINT;

    esp_zb_lock_acquire(portMAX_DELAY);

    /* Send report attributes command */
    esp_zb_zcl_report_attr_cmd_req(&report_attr_cmd);

    esp_zb_lock_release();
    ESP_EARLY_LOGI(TAG, "Sent 'report attributes' command");
}

esp_err_t battery_driver_init()
{
    get_battery_percentage(&battery_percentage, &battery_voltage);

    ESP_LOGI(TAG, "Battery voltage: %.2f V, Battery percentage: %.2f %%", battery_voltage, battery_percentage);

    battery_sensor_save();

    return ESP_OK;
}

void battery_sensor_register_cluster(esp_zb_cluster_list_t *cluster_list)
{
    uint8_t battery_percentage_attr = (uint8_t)battery_percentage;
    uint8_t battery_voltage_attr = (uint8_t)battery_voltage;

    uint8_t battery_size_attr = ESP_ZB_ZCL_POWER_CONFIG_BATTERY_SIZE_BUILT_IN;
    uint8_t battery_rated_voltage = (uint8_t)(BATTERY_VOLTAGE_NOMINAL * 10);
    // add battery % attribute + cluster
    esp_zb_attribute_list_t *esp_zb_power_config_cluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG);

    esp_zb_power_config_cluster_add_attr(esp_zb_power_config_cluster, ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID, &battery_percentage_attr);
    esp_zb_power_config_cluster_add_attr(esp_zb_power_config_cluster, ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_ID, &battery_voltage_attr);
    esp_zb_power_config_cluster_add_attr(esp_zb_power_config_cluster, ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_SIZE_ID, &battery_size_attr);
    esp_zb_power_config_cluster_add_attr(esp_zb_power_config_cluster, ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_ALARM_MASK_ID, ESP_ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_MASK_VOLTAGE_LOW);
    esp_zb_power_config_cluster_add_attr(esp_zb_power_config_cluster, ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_RATED_VOLTAGE_ID, &battery_rated_voltage);

    esp_zb_cluster_list_add_power_config_cluster(cluster_list, esp_zb_power_config_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

    ESP_LOGD(TAG, "Registered zigbee cluster");
}

void battery_sensor_register_reporting_info()
{
    esp_zb_zcl_reporting_info_t reporting_info = {
        .direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV,
        .ep = ZB_ENDPOINT,
        .cluster_id = ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG,
        .cluster_role = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        .dst.profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .u.send_info.min_interval = 1,
        .u.send_info.max_interval = 10,
        .u.send_info.def_min_interval = 1,
        .u.send_info.def_max_interval = 10,
        .u.send_info.delta.u16 = 10,
        .attr_id = ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID,
        .manuf_code = ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC,
    };

    esp_zb_zcl_update_reporting_info(&reporting_info);
}