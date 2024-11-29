#include "illuminance.h"

#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"

static const char *TAG = "I2C";

static esp_err_t BH1750_get_measurement(uint8_t *data_h, uint8_t *data_l)
{
    int ret;
    // make i2c write command
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, BH1750_SENSOR_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, BH1750_CMD_START, ACK_CHECK_EN);
    i2c_master_stop(cmd);

    // send i2c command
    ret = i2c_master_cmd_begin(I2C_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
    {
        return ret;
    }

    // wait for sensor to read
    vTaskDelay(160 / portTICK_PERIOD_MS);

    // make i2c read command
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, BH1750_SENSOR_ADDR << 1 | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, data_h, ACK_VAL);
    i2c_master_read_byte(cmd, data_l, NACK_VAL);
    i2c_master_stop(cmd);

    // send i2c command
    ret = i2c_master_cmd_begin(I2C_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

void get_illuminance_reading(double *lux)
{
    int ret;
    uint8_t sensor_data_h = 0, sensor_data_l = 0;

    // Read the sensor data
    ret = BH1750_get_measurement(&sensor_data_h, &sensor_data_l);

    // Combine sensor data into lux value and update the dereferenced pointer
    *lux = ((sensor_data_h << 8) | sensor_data_l) / 1.2;

    switch (ret)
    {
    case ESP_ERR_TIMEOUT:
        ESP_LOGE(TAG, "I2C Timeout");
        break;
    case ESP_OK:
        ESP_LOGD(TAG, "Received data: 0x%02x%02x", sensor_data_h, sensor_data_l);
        break;
    default:
        ESP_LOGW(TAG, "I2C Error: %s", esp_err_to_name(ret));
        break;
    }
}

esp_err_t BH1750_initiate_i2c(void)
{
    ESP_LOGI(TAG, "initializing i2c");
    int i2c_master_port = I2C_NUM;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ_HZ,
    };
    esp_err_t err = i2c_param_config(i2c_master_port, &conf);
    if (err != ESP_OK)
    {
        return err;
    }

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_RX_BUF_DISABLE, I2C_TX_BUF_DISABLE, 0);
}