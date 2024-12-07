#include "esp_zigbee_core.h"
#include "esp_log.h"
#include "esp_check.h"
#include "ha/esp_zigbee_ha_standard.h"

#include "driver/temp.h"
#include "util/zigbee.h"
#include "temperature.h"
#include "main.h"

static const char *TAG = "Temp_zb";

int16_t temperature = 0;

/* Update temperature sensor measured value */
static void temperature_sensor_save(int16_t temperature)
{
    esp_zb_lock_acquire(portMAX_DELAY);
    esp_zb_zcl_set_attribute_val(ZB_ENDPOINT,
                                 ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                                 ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID, &temperature, false);
    esp_zb_lock_release();
}

void temperature_sensor_report()
{
    esp_zb_zcl_report_attr_cmd_t report_attr_cmd = {0};
    report_attr_cmd.address_mode = ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT;
    report_attr_cmd.attributeID = ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID;
    report_attr_cmd.direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_CLI;
    report_attr_cmd.clusterID = ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT;
    report_attr_cmd.zcl_basic_cmd.src_endpoint = ZB_ENDPOINT;

    esp_zb_lock_acquire(portMAX_DELAY);
    /* Send report attributes command */
    esp_zb_zcl_report_attr_cmd_req(&report_attr_cmd);
    esp_zb_lock_release();
    ESP_EARLY_LOGI(TAG, "Sent 'report attributes' command");
}

void temperature_sensor_register_cluster(esp_zb_cluster_list_t *cluster_list)
{
    get_temperature_readings(&temperature);
    /* Set (Min|Max)MeasuredValure */
    esp_zb_temperature_meas_cluster_cfg_t meas_cfg = {
        .measured_value = temperature,
        .min_value = ((int16_t)TEMP_SENSOR_MIN_VALUE) * 100,
        .max_value = ((int16_t)TEMP_SENSOR_MAX_VALUE) * 100,
    };

    // Add cluster
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_temperature_meas_cluster(
        cluster_list,
        esp_zb_temperature_meas_cluster_create(&meas_cfg),
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));

    ESP_LOGI(TAG, "Registered zigbee cluster");
}

void temperature_sensor_register_reporting_info()
{
    esp_zb_zcl_reporting_info_t reporting_info = {
        .direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV,
        .ep = ZB_ENDPOINT,
        .cluster_id = ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,
        .cluster_role = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        .dst.profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .u.send_info.min_interval = 1,
        .u.send_info.max_interval = 0,
        .u.send_info.def_min_interval = 1,
        .u.send_info.def_max_interval = 0,
        .u.send_info.delta.u16 = 100,
        .attr_id = ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID,
        .manuf_code = ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC,
    };

    esp_zb_zcl_update_reporting_info(&reporting_info);
}