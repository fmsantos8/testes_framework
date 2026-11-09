#ifndef TEST_ESP_SDK_H
#define TEST_ESP_SDK_H
/* Host model of the SDK boundary used by this port, not an ESP-IDF emulator. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef int esp_err_t;
enum { ESP_OK, ESP_FAIL, ESP_ERR_TIMEOUT, ESP_ERR_NO_MEM, ESP_ERR_NOT_FOUND,
       ESP_ERR_INVALID_STATE, ESP_ERR_NOT_SUPPORTED, ESP_ERR_INVALID_ARG };
typedef uint32_t TickType_t;
#define configTICK_RATE_HZ 100U
#define pdMS_TO_TICKS(ms) ((uint32_t)(ms) * configTICK_RATE_HZ / 1000U)
#define portMAX_DELAY UINT32_MAX
#define CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ 160
#define SOC_I2C_NUM 2
#define SOC_UART_NUM 3
#define GPIO_NUM_MAX 40
#define GPIO_IS_VALID_GPIO(n) ((n) < 40 && (n) != 24 && (n) != 28 && (n) != 29 && (n) != 30 && (n) != 31)
#define GPIO_IS_VALID_OUTPUT_GPIO(n) (GPIO_IS_VALID_GPIO(n) && (n) < 34)
#define IRAM_ATTR
typedef int gpio_num_t;
enum { GPIO_MODE_DISABLE, GPIO_MODE_INPUT, GPIO_MODE_OUTPUT,
       GPIO_PULLUP_DISABLE, GPIO_PULLUP_ENABLE, GPIO_PULLDOWN_DISABLE,
       GPIO_PULLDOWN_ENABLE, GPIO_INTR_DISABLE };
typedef struct { uint64_t pin_bit_mask; int mode, pull_up_en, pull_down_en, intr_type; } gpio_config_t;
esp_err_t gpio_config(const gpio_config_t *);
esp_err_t gpio_set_level(gpio_num_t, uint32_t);
int gpio_get_level(gpio_num_t);
esp_err_t gpio_hold_dis(gpio_num_t);
bool esp_gpio_is_reserved(uint64_t);
typedef void *i2c_master_bus_handle_t;
typedef void *i2c_master_dev_handle_t;
typedef struct { int i2c_port, scl_io_num, sda_io_num, clk_source, glitch_ignore_cnt;
    struct { bool enable_internal_pullup; } flags; } i2c_master_bus_config_t;
typedef struct { int dev_addr_length, device_address; uint32_t scl_speed_hz; } i2c_device_config_t;
enum { I2C_CLK_SRC_DEFAULT, I2C_ADDR_BIT_LEN_7 };
esp_err_t i2c_new_master_bus(const i2c_master_bus_config_t *, i2c_master_bus_handle_t *);
esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t, const i2c_device_config_t *, i2c_master_dev_handle_t *);
esp_err_t i2c_master_bus_rm_device(i2c_master_dev_handle_t);
esp_err_t i2c_del_master_bus(i2c_master_bus_handle_t);
esp_err_t i2c_master_get_bus_handle(int, i2c_master_bus_handle_t *);
esp_err_t i2c_master_device_change_address(i2c_master_dev_handle_t, uint16_t, int);
esp_err_t i2c_master_transmit(i2c_master_dev_handle_t, const uint8_t *, size_t, int);
esp_err_t i2c_master_receive(i2c_master_dev_handle_t, uint8_t *, size_t, int);
typedef int uart_port_t;
typedef struct { int baud_rate, data_bits, parity, stop_bits, flow_ctrl, source_clk; } uart_config_t;
enum { UART_DATA_7_BITS, UART_DATA_8_BITS, UART_PARITY_DISABLE, UART_PARITY_EVEN,
       UART_PARITY_ODD, UART_STOP_BITS_1, UART_STOP_BITS_2, UART_HW_FLOWCTRL_DISABLE,
       UART_SCLK_APB, UART_MODE_UART, UART_SIGNAL_INV_DISABLE, UART_PIN_NO_CHANGE };
#define UART_BITRATE_MAX 5000000
#define UART_HW_FIFO_LEN(port) 128
bool uart_is_driver_installed(uart_port_t);
esp_err_t uart_driver_install(uart_port_t, int, int, int, void *, int);
esp_err_t uart_driver_delete(uart_port_t);
esp_err_t uart_param_config(uart_port_t, const uart_config_t *);
esp_err_t uart_set_mode(uart_port_t, int);
esp_err_t uart_set_line_inverse(uart_port_t, uint32_t);
esp_err_t uart_set_pin(uart_port_t, int, int, int, int);
esp_err_t uart_disable_intr_mask(uart_port_t, uint32_t);
int uart_tx_chars(uart_port_t, const char *, uint32_t);
esp_err_t uart_wait_tx_done(uart_port_t, TickType_t);
TickType_t xTaskGetTickCount(void);
void vTaskDelay(TickType_t);
typedef void *gptimer_handle_t;
typedef struct { int unused; } gptimer_alarm_event_data_t;
typedef struct { int clk_src, direction; uint32_t resolution_hz; } gptimer_config_t;
typedef struct { uint64_t alarm_count, reload_count; struct { bool auto_reload_on_alarm; } flags; } gptimer_alarm_config_t;
typedef struct { bool (*on_alarm)(gptimer_handle_t, const gptimer_alarm_event_data_t *, void *); } gptimer_event_callbacks_t;
enum { GPTIMER_CLK_SRC_DEFAULT, GPTIMER_COUNT_UP, SOC_MOD_CLK_CPU, ESP_CLK_TREE_SRC_FREQ_PRECISION_EXACT };
esp_err_t gptimer_new_timer(const gptimer_config_t *, gptimer_handle_t *);
esp_err_t gptimer_get_resolution(gptimer_handle_t, uint32_t *);
esp_err_t gptimer_del_timer(gptimer_handle_t);
esp_err_t gptimer_set_alarm_action(gptimer_handle_t, const gptimer_alarm_config_t *);
esp_err_t gptimer_register_event_callbacks(gptimer_handle_t, const gptimer_event_callbacks_t *, void *);
esp_err_t gptimer_set_raw_count(gptimer_handle_t, uint64_t);
esp_err_t gptimer_enable(gptimer_handle_t);
esp_err_t gptimer_start(gptimer_handle_t);
esp_err_t esp_clk_tree_src_get_freq_hz(int, int, uint32_t *);
#endif
