#include "native_sdk.h"
#include "scenario.h"
#include "application.h"
#include "sht31d.h"
#include "ssd1306.h"
#include <assert.h>
UART_HandleTypeDef huart2;
I2C_HandleTypeDef hi2c1;
i2c_master_dev_handle_t sensor_device = 0x44, display_device = 0x3C;
const struct gpio_dt_spec button = {0}, status_led = {1};
static int map_hal(int status) { return status == IO_OK ? HAL_OK : status == IO_TIMEOUT ? HAL_TIMEOUT : HAL_ERROR; }
static int map_esp(int status) { return status == IO_OK ? ESP_OK : status == IO_TIMEOUT ? ESP_ERR_TIMEOUT : ESP_FAIL; }
static int map_nrf(int status) { return status == IO_OK ? NRFX_SUCCESS : status == IO_TIMEOUT ? NRFX_ERROR_TIMEOUT : NRFX_ERROR_INTERNAL; }
int HAL_GPIO_ReadPin(int port, int pin) { (void)port; (void)pin; return !button_pressed(); }
void HAL_GPIO_WritePin(int port, int pin, int state) { (void)port; (void)pin; led_write(state != 0); }
HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *bus, uint16_t address, const uint8_t *data, uint16_t size, uint32_t timeout) {
    (void)bus; assert(timeout == 100); return map_hal(bus_write(address >> 1,data,size));
}
HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *bus, uint16_t address, uint8_t *data, uint16_t size, uint32_t timeout) {
    (void)bus; assert(timeout == 100); return map_hal(bus_read(address >> 1,data,size));
}
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *uart, const uint8_t *data, uint16_t size, uint32_t timeout) {
    assert(timeout == 1000); uint32_t count;
    int status = uart_write(data,size,&count); uart->TxXferCount = (uint16_t)(size-count); return map_hal(status);
}
int gpio_get_level(int pin) { (void)pin; return !button_pressed(); }
esp_err_t gpio_set_level(int pin, int level) { (void)pin; led_write(level != 0); return ESP_OK; }
esp_err_t i2c_master_transmit(int address, const uint8_t *data, size_t size, int timeout) {
    assert(timeout == 100); return map_esp(bus_write((uint8_t)address,data,(uint32_t)size));
}
esp_err_t i2c_master_receive(int address, uint8_t *data, size_t size, int timeout) {
    assert(timeout == 100); return map_esp(bus_read((uint8_t)address,data,(uint32_t)size));
}
int gpio_pin_get_dt(const struct gpio_dt_spec *pin) { (void)pin; return button_pressed(); }
int gpio_pin_set_dt(const struct gpio_dt_spec *pin, int value) { (void)pin; led_write(value != 0); return 0; }
nrfx_err_t twim_transfer(uint8_t address, uint8_t *data, uint32_t size, bool read) {
    return map_nrf(read ? bus_read(address,data,size) : bus_write(address,data,size));
}
nrfx_err_t uarte_transmit(const uint8_t *data, uint32_t size, uint32_t *count) { return map_nrf(uart_write(data,size,count)); }
/* ESP FIFO staging: the SDK can accept a prefix, then report an error or time out. */
static bool esp_active;
static uint32_t esp_remaining;
static int esp_result;
static TickType_t scheduler_ticks;
TickType_t xTaskGetTickCount(void) { return scheduler_ticks++; }
void vTaskDelay(uint32_t ticks) { scheduler_ticks += ticks; }
int uart_tx_chars(int port, const char *data, uint32_t size) {
    (void)port;
    if (!esp_active) { esp_active = true; esp_result = uart_write((const uint8_t *)data,size,&esp_remaining); }
    if (esp_remaining) { int accepted = (int)esp_remaining; esp_remaining = 0; return accepted; }
    return esp_result == IO_TIMEOUT ? 0 : -1;
}
esp_err_t uart_wait_tx_done(int port, uint32_t ticks) { (void)port; (void)ticks; return map_esp(esp_result); }
void firmware_init(void) {
    led_write(false); /* Native main configures the inactive output before devices. */
    assert(sht31d_init() == SHT31D_STATUS_OK);
    assert(ssd1306_init() == SSD1306_STATUS_OK);
    application_init();
}
void firmware_process(void) { esp_active = false; application_process(); }
