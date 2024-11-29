// Config
#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)

#include "sdkconfig.h"
#include "esp_err.h"

#define BH1750_SENSOR_ADDR CONFIG_BH1750_ADDR /*!< slave address for BH1750 sensor */
#define BH1750_CMD_START CONFIG_BH1750_OPMODE /*!< Operation mode */
#define WRITE_BIT I2C_MASTER_WRITE            /*!< I2C write */
#define READ_BIT I2C_MASTER_READ              /*!< I2C read */
#define ACK_CHECK_EN 0x1                      /*!< I2C will check ack from slave*/
#define ACK_CHECK_DIS 0x0                     /*!< I2C will not check ack from slave */
#define ACK_VAL 0x0                           /*!< I2C ack value */
#define NACK_VAL 0x1                          /*!< I2C nack value */

#define I2C_SCL_IO CONFIG_I2C_SCL               /*!< gpio number for I2C clock */
#define I2C_SDA_IO CONFIG_I2C_SDA               /*!< gpio number for I2C data  */
#define I2C_NUM I2C_NUMBER(CONFIG_I2C_PORT_NUM) /*!< I2C port number for dev */
#define I2C_FREQ_HZ CONFIG_I2C_FREQUENCY        /*!< I2C clock frequency */
#define I2C_TX_BUF_DISABLE 0                    /*!< I2C doesn't need buffer */
#define I2C_RX_BUF_DISABLE 0

// Functions

esp_err_t BH1750_initiate_i2c(void);
void get_illuminance_reading(double *lux);