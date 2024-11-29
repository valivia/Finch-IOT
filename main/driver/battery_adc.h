#define ADC_CHANNEL 0
#define ADC_MAX_VALUE 4095.0 // 12-bit ADC
#define ADC_REFERENCE_VOLTAGE 3.3

#define VOLTAGE_DIVIDER_R1 20000.0
#define VOLTAGE_DIVIDER_R2 20000.0

#define BATTERY_VOLTAGE_MIN 2.5     // minimum voltage
#define BATTERY_VOLTAGE_NOMINAL 3.7 // nominal
#define BATTERY_VOLTAGE_MAX 4.2     // max

// Functions
void get_battery_percentage(float *battery_voltage, float *battery_percentage);