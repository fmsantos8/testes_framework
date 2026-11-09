#ifndef NATIVE_SDK_H
#define NATIVE_SDK_H
/* Test-only SDK boundary; production builds use the real vendor headers. */
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
typedef int HAL_StatusTypeDef;
typedef struct { uint16_t TxXferCount; } UART_HandleTypeDef;
typedef struct { int unused; } I2C_HandleTypeDef;
extern UART_HandleTypeDef huart2;
extern I2C_HandleTypeDef hi2c1;
#define HAL_OK 0
#define HAL_ERROR 1
#define HAL_TIMEOUT 3
#define GPIO_PIN_RESET 0
#define GPIO_PIN_SET 1
#define BUTTON_GPIO_Port 0
#define BUTTON_Pin 0
#define LED_GPIO_Port 0
#define LED_Pin 1
int HAL_GPIO_ReadPin(int port, int pin);
void HAL_GPIO_WritePin(int port, int pin, int state);
HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *, uint16_t, const uint8_t *, uint16_t, uint32_t);
HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *, uint16_t, uint8_t *, uint16_t, uint32_t);
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *, const uint8_t *, uint16_t, uint32_t);
typedef int esp_err_t;
typedef int i2c_master_dev_handle_t;
extern i2c_master_dev_handle_t sensor_device, display_device;
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_TIMEOUT 0x107
#define BUTTON_PIN 18
#define LED_PIN 19
#define UART_NUM_0 0
int gpio_get_level(int pin);
esp_err_t gpio_set_level(int pin, int level);
esp_err_t i2c_master_transmit(int, const uint8_t *, size_t, int);
esp_err_t i2c_master_receive(int, uint8_t *, size_t, int);
typedef uint32_t TickType_t;
#define pdMS_TO_TICKS(x) (x)
TickType_t xTaskGetTickCount(void);
void vTaskDelay(uint32_t ticks);
int uart_tx_chars(int port, const char *data, uint32_t size);
esp_err_t uart_wait_tx_done(int port, uint32_t ticks);
typedef int nrfx_err_t;
#define NRFX_SUCCESS 0
#define NRFX_ERROR_INTERNAL 1
#define NRFX_ERROR_TIMEOUT 2
#define NRFX_ERROR_INVALID_STATE 3
struct gpio_dt_spec { int pin; };
extern const struct gpio_dt_spec button, status_led;
int gpio_pin_get_dt(const struct gpio_dt_spec *);
int gpio_pin_set_dt(const struct gpio_dt_spec *, int);
nrfx_err_t twim_transfer(uint8_t, uint8_t *, uint32_t, bool);
nrfx_err_t uarte_transmit(const uint8_t *, uint32_t, uint32_t *);
#endif
