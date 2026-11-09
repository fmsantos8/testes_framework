#include "scenario.h"
#include <assert.h>
#include "cmp_button_config.h"
#include "cmp_led_config.h"
#include "cmp_sht31d_config.h"
#include "cmp_ssd1306_config.h"
#include "cmp_telemetry_config.h"
#include "acquisition/app_acquisition.h"
#include "button/app_button.h"
#include "gui/app_gui.h"
#include "status_led/app_status_led.h"
#include "telemetry/app_telemetry.h"
struct hal_i2c { int unused; };
struct hal_uart { int unused; };
struct hal_gpio { int unused; };
static struct hal_i2c bus;
static struct hal_uart uart;
static struct hal_gpio led_pin, button_pin;
static led_t led;
static button_t button;
static sht31d_t sensor;
static ssd1306_t display;
static telemetry_t telemetry;
static uint32_t ticks(void) { return app_ticks; }
static hal_status_t translate(int status) { return status == IO_OK ? HAL_STATUS_OK : status == IO_TIMEOUT ? HAL_STATUS_TIMEOUT : HAL_STATUS_ERROR; }
hal_status_t hal_gpio_read(hal_gpio_t *gpio, bool *level) { (void)gpio; *level = !button_pressed(); return HAL_STATUS_OK; }
hal_status_t hal_gpio_write(hal_gpio_t *gpio, bool level) { (void)gpio; led_write(level); return HAL_STATUS_OK; }
hal_status_t hal_i2c_write(hal_i2c_t *i2c, uint8_t address, const uint8_t *data, uint32_t size) { (void)i2c; return translate(bus_write(address,data,size)); }
hal_status_t hal_i2c_read(hal_i2c_t *i2c, uint8_t address, uint8_t *data, uint32_t size) { (void)i2c; return translate(bus_read(address,data,size)); }
hal_status_t hal_uart_write(hal_uart_t *uart_handle, const uint8_t *data, uint32_t size, uint32_t *count) {
    (void)uart_handle; *count = 0;
    return size ? translate(uart_write(data,size,count)) : HAL_STATUS_OK;
}
void firmware_init(void) {
    const led_config_t lc = {.gpio=&led_pin, .active_level=true};
    assert(cmp_led_init(&led,&lc) == LED_STATUS_OK);
    const telemetry_config_t tc = {.uart=&uart};
    assert(cmp_telemetry_init(&telemetry,&tc) == TELEMETRY_STATUS_OK);
    const button_config_t bc = {.gpio=&button_pin,.active_level=false,.get_ticks=ticks};
    assert(cmp_button_init(&button,&bc) == BUTTON_STATUS_OK);
    const sht31d_config_t sc = {.i2c=&bus,.address=0x44};
    assert(cmp_sht31d_init(&sensor,&sc) == SHT31D_STATUS_OK);
    const ssd1306_config_t dc = {.i2c=&bus,.address=0x3C};
    assert(cmp_ssd1306_init(&display,&dc) == SSD1306_STATUS_OK);
    assert(app_status_led_init(ticks)); assert(app_telemetry_init(ticks));
    assert(app_acquisition_init(ticks)); assert(app_gui_init(ticks)); app_button_init();
    app_status_led_set_ready(true);
}
void firmware_process(void) {
    app_button_run(&button); app_acquisition_run(&sensor); app_gui_run(&display);
    app_status_led_run(&led); app_telemetry_run(&telemetry);
}
