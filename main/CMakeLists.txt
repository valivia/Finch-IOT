idf_component_register(
    SRCS "main.c" "driver/illuminance.c" "driver/temp.c" "driver/switch_driver.c" "util/zigbee.c" "clusters/temperature.c" "clusters/light.c"
    INCLUDE_DIRS "."
    REQUIRES "esp-zboss-lib" "esp-zigbee-lib" "nvs_flash" "freertos" "esp_timer"
)
