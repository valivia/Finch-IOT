#include "esp_check.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/rtc_io.h"
#include "time.h"
#include "esp_sleep.h"
#include "sys/time.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ha/esp_zigbee_ha_standard.h"

#include "main.h"

// Clusters
#include "clusters/light.h"
#include "clusters/battery.h"

// Sensor drivers
#include "driver/switch_driver.h"
#include "driver/illuminance.h"
#include "driver/battery_adc.h"

#include "util/zigbee.h"

static const char *TAG = "Main";

static RTC_DATA_ATTR struct timeval s_sleep_enter_time;
static esp_timer_handle_t s_oneshot_timer;

static int wakeup_time_sec = IS_PRODUCTION ? 5 * 60 : 10;

static bool first_boot = false;
static bool keep_on = false;
static uint8_t join_attempts = 0;

#define SENSOR_READINGS_COUNT 10
static RTC_DATA_ATTR uint16_t light_sensor_readings[SENSOR_READINGS_COUNT] = {-1, -1, -1, -1, -1};
static RTC_DATA_ATTR uint16_t current_sensor_reading_index = 0;

static int get_new_index(int change, int current_index, int array_size)
{
    int new_index = (current_index + change) % array_size;
    if (new_index < 0)
    {
        new_index += array_size;
    }
    return new_index;
}

// Sleep
static void deep_sleep_start(void *arg)
{
    if (keep_on)
    {
        ESP_LOGI(TAG, "Keep on, not entering deep sleep");
        return;
    }

    // Start deep sleep timer
    ESP_LOGI(TAG, "Entering deep sleep for %ds\n", wakeup_time_sec);
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000));

    // Store the time when entering deep sleep
    gettimeofday(&s_sleep_enter_time, NULL);

    // Turn off LED
    gpio_set_direction(GPIO_MODE_DISABLE, LED_PIN);

    // Increment sensor reading index
    current_sensor_reading_index = get_new_index(1, current_sensor_reading_index, SENSOR_READINGS_COUNT);

    // Enter deep sleep
    esp_deep_sleep_start();
}

static void zb_deep_sleep_init(void)
{
    const esp_timer_create_args_t s_oneshot_timer_args = {
        .callback = &deep_sleep_start,
        .name = "one-shot"};

    ESP_ERROR_CHECK(esp_timer_create(&s_oneshot_timer_args, &s_oneshot_timer));

    // Print the wake-up reason:
    struct timeval now;
    gettimeofday(&now, NULL);
    int sleep_time_ms = (now.tv_sec - s_sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - s_sleep_enter_time.tv_usec) / 1000;
    esp_sleep_wakeup_cause_t wake_up_cause = esp_sleep_get_wakeup_cause();
    switch (wake_up_cause)
    {
    case ESP_SLEEP_WAKEUP_TIMER:
    {
        ESP_LOGI(TAG, "Wake up from timer. Time spent in deep sleep and boot: %dms", sleep_time_ms);
        break;
    }
    case ESP_SLEEP_WAKEUP_EXT1:
    {
        ESP_LOGI(TAG, "Wake up from GPIO. Time spent in deep sleep and boot: %dms", sleep_time_ms);
        break;
    }
    case ESP_SLEEP_WAKEUP_UNDEFINED:
    default:
        ESP_LOGI(TAG, "Not a deep sleep reset");
        break;
    }
}

static void deep_sleep_timer_start(void)
{
    int before_deep_sleep_time_sec = 5;

    if (first_boot)
    {
        first_boot = false;
        before_deep_sleep_time_sec = 120;
    }

    ESP_LOGI(TAG, "Start one-shot timer for %ds to enter the deep sleep", before_deep_sleep_time_sec);
    ESP_ERROR_CHECK(esp_timer_start_once(s_oneshot_timer, before_deep_sleep_time_sec * 1000000));
}

// Button
static switch_func_pair_t button_func_pair[] = {
    {GPIO_INPUT_IO_TOGGLE_SWITCH, SWITCH_ONOFF_TOGGLE_CONTROL}};

static void esp_app_buttons_handler(switch_func_pair_t *button_func_pair)
{
    if (button_func_pair->func == SWITCH_ONOFF_TOGGLE_CONTROL)
    {
        if (keep_on)
        {
            deep_sleep_timer_start();
        }

        keep_on = !keep_on;
    }
}

static void bdb_start_top_level_commissioning_cb(uint8_t mode_mask)
{
    ESP_RETURN_ON_FALSE(esp_zb_bdb_start_top_level_commissioning(mode_mask) == ESP_OK, ,
                        TAG, "Failed to start Zigbee bdb commissioning");
}

// Main
void report_all_sensors()
{
    light_sensor_report();
    battery_sensor_report();
}

void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    uint32_t *p_sg_p = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    esp_zb_app_signal_type_t sig_type = *p_sg_p;
    switch (sig_type)
    {
    case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
        ESP_LOGI(TAG, "Initialize Zigbee stack");
        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
        break;
    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
        ESP_LOGI(TAG, "Device started up in %s factory-reset mode", esp_zb_bdb_is_factory_new() ? "" : "non");

        if (esp_zb_bdb_is_factory_new())
            first_boot = true;

        if (err_status == ESP_OK)
        {
            if (esp_zb_bdb_is_factory_new())
            {
                ESP_LOGI(TAG, "Start network steering");
                esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
            }
            else
            {
                report_all_sensors();
                deep_sleep_timer_start();
            }
        }
        else
        {
            /* commissioning failed */
            ESP_LOGW(TAG, "Failed to initialize Zigbee stack (status: %s)", esp_err_to_name(err_status));

            if (join_attempts < 3)
            {
                join_attempts++;
                ESP_LOGI(TAG, "Retry network steering");
                esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb, ESP_ZB_BDB_MODE_NETWORK_STEERING, 1000);
            }
            else
            {
                ESP_LOGW(TAG, "Join attempts exceeded, enter deep sleep");
                deep_sleep_start(NULL);
            }
        }
        break;
    case ESP_ZB_BDB_SIGNAL_STEERING:
        if (err_status == ESP_OK)
        {
            esp_zb_ieee_addr_t extended_pan_id;
            esp_zb_get_extended_pan_id(extended_pan_id);
            ESP_LOGI(TAG, "Joined network successfully (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN ID: 0x%04hx, Channel:%d, Short Address: 0x%04hx)",
                     extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                     extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0],
                     esp_zb_get_pan_id(), esp_zb_get_current_channel(), esp_zb_get_short_address());
            deep_sleep_timer_start();
        }
        else
        {
            ESP_LOGI(TAG, "Network steering was not successful (status: %s)", esp_err_to_name(err_status));
            esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb, ESP_ZB_BDB_MODE_NETWORK_STEERING, 1000);
        }
        break;
    default:
        ESP_LOGI(TAG, "ZDO signal: %s (0x%x), status: %s", esp_zb_zdo_signal_to_string(sig_type), sig_type,
                 esp_err_to_name(err_status));
        break;
    }
}

static void esp_zb_task(void *pvParameters)
{
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // Read all sensors
    battery_driver_init();
    illuminance_driver_init();
    switch_driver_init(button_func_pair, PAIR_SIZE(button_func_pair), esp_app_buttons_handler);

    // Store illuminance reading in array;
    get_illuminance_reading(&light_sensor_readings[current_sensor_reading_index]);

    bool same_as_previous = light_sensor_readings[current_sensor_reading_index] == light_sensor_readings[get_new_index(-1, current_sensor_reading_index, SENSOR_READINGS_COUNT)];

    // If the illuminance readings are the same as the previous reading, enter deep sleep
    if (same_as_previous)
    {
        bool is_trend = light_sensor_readings[current_sensor_reading_index] == light_sensor_readings[get_new_index(-2, current_sensor_reading_index, SENSOR_READINGS_COUNT)];

        if (is_trend)
            wakeup_time_sec = 15 * 60;

        ESP_LOGI(TAG, "Illuminance readings are the same, entering %s deep sleep", is_trend ? "extended" : "normal");
        deep_sleep_start(NULL);
        return;
    }

    /* Initialize Zigbee stack */
    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZED_CONFIG();
    esp_zb_init(&zb_nwk_cfg);

    /* Register endpoints */
    set_endpoints();

    /* Config the reporting info  */
    set_reporting();

    ESP_LOGI(TAG, "All endpoints registered");

    esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK);
    ESP_ERROR_CHECK(esp_zb_start(false));

    ESP_LOGI(TAG, "Zigbee stack initialized");

    esp_zb_stack_main_loop();
}

void app_main(void)
{
    esp_zb_platform_config_t config = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_ZB_DEFAULT_HOST_CONFIG(),
    };

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_zb_platform_config(&config));

    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);

    zb_deep_sleep_init();

    /* Start Zigbee stack task */
    xTaskCreate(esp_zb_task, "Zigbee_main", 4096, NULL, 5, NULL);
}
