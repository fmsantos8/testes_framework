#ifndef CONFORMANCE_FIXTURE_H
#define CONFORMANCE_FIXTURE_H
#include <stdbool.h>
#include <stdint.h>
#include "system/hal_system.h"
#include "gpio/hal_gpio.h"
#include "i2c/hal_i2c.h"
#include "uart/hal_uart.h"
#include "timer/hal_timer.h"

/* Test controls affect the SDK boundary, never replace public HAL functions. */
typedef enum { F_OK, F_ERROR, F_TIMEOUT, F_PARTIAL } fixture_mode_t;
extern fixture_mode_t fixture_bus_mode, fixture_uart_mode;
extern bool fixture_bus_init_failure;
extern uint8_t fixture_last_address;
extern uint32_t fixture_last_timeout;
extern hal_gpio_config_t fixture_input, fixture_output;
extern hal_i2c_config_t fixture_bus;
extern hal_uart_config_t fixture_serial;
extern hal_timer_config_t fixture_timer;
extern uint32_t fixture_cpu;
extern uint32_t fixture_i2c_limit;
void fixture_setup(void);
void fixture_set_input(bool level);
bool fixture_output_level(void);
void fixture_fire_timer(void);
bool fixture_unsupported_timer(hal_timer_config_t *config);
#endif
