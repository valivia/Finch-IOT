idf_component_register(
    SRCS 
    "main.c"
    "util/zigbee.c"
    "driver/temp.c" "driver/illuminance.c" "driver/battery_adc.c" "driver/switch_driver.c"  
    "clusters/temperature.c" "clusters/light.c" "clusters/battery.c"
    INCLUDE_DIRS "."
    REQUIRES "esp-zboss-lib" "esp-zigbee-lib" "nvs_flash" "freertos" "esp_timer" "esp_adc"
)
