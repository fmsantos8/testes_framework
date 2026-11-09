#include "main.h"
#include "application.h"
#include "sht31d.h"
#include "ssd1306.h"
#include "driver/gptimer.h"
#include "driver/uart.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdlib.h>
volatile uint32_t app_ticks;
i2c_master_dev_handle_t sensor_device, display_device;
static gptimer_handle_t timer;

static bool IRAM_ATTR timer_alarm(gptimer_handle_t handle, const gptimer_alarm_event_data_t *event, void *context) {
    (void)handle;
    (void)event;
    (void)context;
    ++app_ticks;
    return false;
}
static void init_gpio(void) {
    ESP_ERROR_CHECK(gpio_set_level(LED_PIN, 0));
    const gpio_config_t led = {.pin_bit_mask = UINT64_C(1) << LED_PIN, .mode = GPIO_MODE_OUTPUT};
    ESP_ERROR_CHECK(gpio_config(&led));
    const gpio_config_t button = {.pin_bit_mask = UINT64_C(1) << BUTTON_PIN, .mode = GPIO_MODE_INPUT};
    ESP_ERROR_CHECK(gpio_config(&button));
}
static void init_i2c(void) {
    i2c_master_bus_handle_t bus;
    const i2c_master_bus_config_t config = {.i2c_port = 0,
                                            .scl_io_num = GPIO_NUM_22,
                                            .sda_io_num = GPIO_NUM_21,
                                            .clk_source = I2C_CLK_SRC_DEFAULT,
                                            .glitch_ignore_cnt = 7,
                                            .flags.enable_internal_pullup = false};
    ESP_ERROR_CHECK(i2c_new_master_bus(&config, &bus));
    i2c_device_config_t device = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7, .device_address = 0x44, .scl_speed_hz = 100000};
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &device, &sensor_device));
    device.device_address = 0x3C;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &device, &display_device));
}
static void init_uart(void) {
    const uart_config_t config = {.baud_rate = 115200,
                                  .data_bits = UART_DATA_8_BITS,
                                  .parity = UART_PARITY_DISABLE,
                                  .stop_bits = UART_STOP_BITS_1,
                                  .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                                  .source_clk = UART_SCLK_APB};
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, UART_HW_FIFO_LEN(UART_NUM_0) + 1, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &config));
    ESP_ERROR_CHECK(uart_set_mode(UART_NUM_0, UART_MODE_UART));
    ESP_ERROR_CHECK(uart_set_line_inverse(UART_NUM_0, UART_SIGNAL_INV_DISABLE));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, GPIO_NUM_1, GPIO_NUM_3, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_disable_intr_mask(UART_NUM_0, UINT32_MAX));
}
static void init_timer(void) {
    const gptimer_config_t config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT, .direction = GPTIMER_COUNT_UP, .resolution_hz = 1000000};
    ESP_ERROR_CHECK(gptimer_new_timer(&config, &timer));
    const gptimer_alarm_config_t alarm = {.alarm_count = 1000, .reload_count = 0, .flags.auto_reload_on_alarm = true};
    ESP_ERROR_CHECK(gptimer_set_alarm_action(timer, &alarm));
    const gptimer_event_callbacks_t callbacks = {.on_alarm = timer_alarm};
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer, &callbacks, NULL));
    ESP_ERROR_CHECK(gptimer_set_raw_count(timer, 0));
    ESP_ERROR_CHECK(gptimer_enable(timer));
}
void app_main(void) {
    init_gpio();
    init_i2c();
    init_uart();
    init_timer();
    if (sht31d_init() != SHT31D_STATUS_OK || ssd1306_init() != SSD1306_STATUS_OK)
        abort();
    application_init();
    ESP_ERROR_CHECK(gptimer_start(timer));
    while (true) {
        application_process();
    }
}
