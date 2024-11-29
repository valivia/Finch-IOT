#include "esp_zigbee_core.h"
#include "esp_log.h"
#include "esp_check.h"
#include "ha/esp_zigbee_ha_standard.h"

#include "light.h"
#include "driver/illuminance.h"
#include "util/zigbee.h"
#include "main.h"

static const char *TAG = "illuminance_zb";

/* Update light sensor measured value */
static void light_sensor_save(uint16_t value)
{
    esp_zb_lock_acquire(portMAX_DELAY);
    esp_zb_zcl_set_attribute_val(ZB_ENDPOINT,
                                 ESP_ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                                 ESP_ZB_ZCL_ATTR_ILLUMINANCE_MEASUREMENT_MEASURED_VALUE_ID, &value, false);
    esp_zb_lock_release();
}

void light_sensor_report()
{
    esp_zb_zcl_report_attr_cmd_t report_attr_cmd = {0};
    report_attr_cmd.address_mode = ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT;
    report_attr_cmd.attributeID = ESP_ZB_ZCL_ATTR_ILLUMINANCE_MEASUREMENT_MEASURED_VALUE_ID;
    report_attr_cmd.direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_CLI;
    report_attr_cmd.clusterID = ESP_ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT;
    report_attr_cmd.zcl_basic_cmd.src_endpoint = ZB_ENDPOINT;

    esp_zb_lock_acquire(portMAX_DELAY);

    /* Send report attributes command */
    esp_zb_zcl_report_attr_cmd_req(&report_attr_cmd);

    esp_zb_lock_release();
    ESP_EARLY_LOGI(TAG, "Sent 'report attributes' command");
}

static void light_sensor_task(void *pvParameters)
{
    while (1)
    {
        double value = 0;
        /* Read light sensor value */
        get_illuminance_reading(&value);

        /* Save light sensor value */
        uint16_t measured_value = 10000 * log10(value + 1);
        ESP_LOGD(TAG, "value: %.02f, measured_value: %d", value, measured_value);
        light_sensor_save(measured_value);

        vTaskDelay(LIGHT_SENSOR_UPDATE_INTERVAL * 1000 / portTICK_PERIOD_MS);
    }
}

esp_err_t light_driver_init(void)
{
    ESP_RETURN_ON_ERROR(BH1750_initiate_i2c(), TAG, "Failed to initialize light sensor");
    xTaskCreate(light_sensor_task, "light_read", 4096, NULL, 1, NULL);
    ESP_LOGI(TAG, "Light sensor driver initialized");
    return ESP_OK;
}

void light_sensor_register_cluster(esp_zb_cluster_list_t *cluster_list)
{
    /* Set (Min|Max)MeasuredValure */
    esp_zb_illuminance_meas_cluster_cfg_t meas_cfg = {
        .measured_value = LIGHT_SENSOR_MAX_VALUE,
        .min_value = LIGHT_SENSOR_MIN_VALUE,
        .max_value = LIGHT_SENSOR_MAX_VALUE,
    };

    // Add cluster
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_illuminance_meas_cluster(
        cluster_list,
        esp_zb_illuminance_meas_cluster_create(&meas_cfg),
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));

    ESP_LOGD(TAG, "Registered zigbee cluster");
}

void light_sensor_register_reporting_info()
{
    esp_zb_zcl_reporting_info_t reporting_info = {
        .direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV,
        .ep = ZB_ENDPOINT,
        .cluster_id = ESP_ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT,
        .cluster_role = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        .dst.profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .u.send_info.min_interval = 1,
        .u.send_info.max_interval = 10,
        .u.send_info.def_min_interval = 1,
        .u.send_info.def_max_interval = 10,
        .u.send_info.delta.u16 = 1,
        .attr_id = ESP_ZB_ZCL_ATTR_ILLUMINANCE_MEASUREMENT_MEASURED_VALUE_ID,
        .manuf_code = ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC,
    };

    esp_zb_zcl_update_reporting_info(&reporting_info);
}