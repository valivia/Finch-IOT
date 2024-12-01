#define ADC_CHANNEL ADC_CHANNEL_2
#define ADC_ATTEN ADC_ATTEN_DB_12
#define ADC_BITWIDTH ADC_BITWIDTH_12

#define BATTERY_READ_PIN 21

#define ADC_MAX_VALUE 4095.0 // 12-bit ADC
#define ADC_REFERENCE_VOLTAGE 3.3

#define VOLTAGE_DIVIDER_R1 20000.0
#define VOLTAGE_DIVIDER_R2 20000.0

#define BATTERY_VOLTAGE_MIN_MILLIVOLT 2500     // minimum voltage
#define BATTERY_VOLTAGE_NOMINAL_MILLIVOLT 3700 // nominal
#define BATTERY_VOLTAGE_MAX_MILLIVOLT 4200     // max

// Functions
esp_err_t battery_driver_init();
void get_battery_readings(uint16_t *battery_voltage, double *battery_percentage);
