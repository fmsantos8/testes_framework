#include "fixture.h"
#include "sdk.h"
#include <assert.h>
#include <string.h>
static bool levels[40], installed[3];
static uint32_t ticks, uart_accepted;
static gptimer_event_callbacks_t callbacks;
static void *timer_context;
void fixture_setup(void) {
    fixture_cpu = 160000000; fixture_i2c_limit = UINT16_MAX;
    fixture_input = (hal_gpio_config_t){{0,18}, HAL_GPIO_INPUT, HAL_GPIO_PULL_NONE, false};
    fixture_output = (hal_gpio_config_t){{0,19}, HAL_GPIO_OUTPUT, HAL_GPIO_PULL_NONE, false};
    fixture_bus = (hal_i2c_config_t){0,{0,22},{0,21},100000,100};
    fixture_serial = (hal_uart_config_t){0,{0,1},{0,3},115200,HAL_UART_DATA_WIDTH_8,HAL_UART_PARITY_NONE,HAL_UART_STOP_COUNT_1,1000};
    fixture_timer = (hal_timer_config_t){.instance=0,.period=1};
}
void fixture_set_input(bool level) { levels[18] = level; }
bool fixture_output_level(void) { return levels[19]; }
void fixture_fire_timer(void) { assert(callbacks.on_alarm); callbacks.on_alarm((void *)1,NULL,timer_context); }
bool fixture_unsupported_timer(hal_timer_config_t *config) { (void)config; return false; }
esp_err_t gpio_config(const gpio_config_t *config) { (void)config; return ESP_OK; }
esp_err_t gpio_set_level(gpio_num_t pin, uint32_t level) { levels[pin] = level != 0; return ESP_OK; }
int gpio_get_level(gpio_num_t pin) { return levels[pin]; }
esp_err_t gpio_hold_dis(gpio_num_t pin) { (void)pin; return ESP_OK; }
bool esp_gpio_is_reserved(uint64_t mask) { (void)mask; return false; }
esp_err_t esp_clk_tree_src_get_freq_hz(int source,int precision,uint32_t *value) { (void)source;(void)precision;*value=160000000;return ESP_OK; }
esp_err_t i2c_new_master_bus(const i2c_master_bus_config_t *config,i2c_master_bus_handle_t *out) {
    assert(!config->flags.enable_internal_pullup);
    if(fixture_bus_init_failure)return ESP_FAIL;
    *out=(void *)1;return ESP_OK;
}
esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t bus,const i2c_device_config_t *config,i2c_master_dev_handle_t *out) {(void)bus;(void)config;*out=(void *)2;return ESP_OK;}
esp_err_t i2c_master_bus_rm_device(i2c_master_dev_handle_t device) {(void)device;return ESP_OK;}
esp_err_t i2c_del_master_bus(i2c_master_bus_handle_t bus) {(void)bus;return ESP_OK;}
esp_err_t i2c_master_get_bus_handle(int port,i2c_master_bus_handle_t *out) {(void)port;*out=NULL;return ESP_ERR_NOT_FOUND;}
esp_err_t i2c_master_device_change_address(i2c_master_dev_handle_t device,uint16_t address,int timeout) {(void)device;fixture_last_address=address;fixture_last_timeout=timeout;return ESP_OK;}
static esp_err_t transfer_result(void) {return fixture_bus_mode==F_OK?ESP_OK:fixture_bus_mode==F_TIMEOUT?ESP_ERR_TIMEOUT:ESP_FAIL;}
esp_err_t i2c_master_transmit(i2c_master_dev_handle_t device,const uint8_t *data,size_t size,int timeout) {(void)device;(void)data;(void)size;fixture_last_timeout=timeout;return transfer_result();}
esp_err_t i2c_master_receive(i2c_master_dev_handle_t device,uint8_t *data,size_t size,int timeout) {(void)device;fixture_last_timeout=timeout;if(fixture_bus_mode==F_OK)memset(data,0x5a,size);return transfer_result();}
bool uart_is_driver_installed(uart_port_t port) {return installed[port];}
esp_err_t uart_driver_install(uart_port_t port,int rx,int tx,int events,void *queue,int flags) {(void)rx;(void)events;(void)queue;(void)flags;assert(tx==0);installed[port]=true;return ESP_OK;}
esp_err_t uart_driver_delete(uart_port_t port) {installed[port]=false;return ESP_OK;}
esp_err_t uart_param_config(uart_port_t port,const uart_config_t *config) {(void)port;assert(config->baud_rate==115200);return ESP_OK;}
esp_err_t uart_set_mode(uart_port_t port,int mode) {(void)port;(void)mode;return ESP_OK;}
esp_err_t uart_set_line_inverse(uart_port_t port,uint32_t value) {(void)port;(void)value;return ESP_OK;}
esp_err_t uart_set_pin(uart_port_t port,int tx,int rx,int rts,int cts) {(void)port;(void)tx;(void)rx;(void)rts;(void)cts;return ESP_OK;}
esp_err_t uart_disable_intr_mask(uart_port_t port,uint32_t mask) {(void)port;(void)mask;return ESP_OK;}
int uart_tx_chars(uart_port_t port,const char *data,uint32_t size) {
    (void)port;(void)data;
    if(fixture_uart_mode==F_ERROR)return -1;
    if(fixture_uart_mode==F_TIMEOUT)return 0;
    if(fixture_uart_mode==F_PARTIAL){if(uart_accepted)return 0;uart_accepted=2;return 2;}
    return (int)size;
}
esp_err_t uart_wait_tx_done(uart_port_t port,TickType_t limit) {(void)port;(void)limit;return ESP_OK;}
TickType_t xTaskGetTickCount(void) {return ticks++;}
void vTaskDelay(TickType_t delay) {ticks+=delay;}
esp_err_t gptimer_new_timer(const gptimer_config_t *config,gptimer_handle_t *out) {assert(config->resolution_hz==1000000);*out=(void *)1;return ESP_OK;}
esp_err_t gptimer_get_resolution(gptimer_handle_t timer,uint32_t *value) {(void)timer;*value=1000000;return ESP_OK;}
esp_err_t gptimer_del_timer(gptimer_handle_t timer) {(void)timer;return ESP_OK;}
esp_err_t gptimer_set_alarm_action(gptimer_handle_t timer,const gptimer_alarm_config_t *alarm) {(void)timer;assert(alarm->alarm_count==1000&&alarm->flags.auto_reload_on_alarm);return ESP_OK;}
esp_err_t gptimer_register_event_callbacks(gptimer_handle_t timer,const gptimer_event_callbacks_t *config,void *context) {(void)timer;callbacks=*config;timer_context=context;return ESP_OK;}
esp_err_t gptimer_set_raw_count(gptimer_handle_t timer,uint64_t value) {(void)timer;assert(value==0);return ESP_OK;}
esp_err_t gptimer_enable(gptimer_handle_t timer) {(void)timer;return ESP_OK;}
esp_err_t gptimer_start(gptimer_handle_t timer) {(void)timer;return ESP_OK;}
