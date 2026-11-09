#ifndef MAIN_H
#define MAIN_H
#include <stdint.h>
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <nrfx_twim.h>
#include <nrfx_uarte.h>
extern const struct gpio_dt_spec button;
extern const struct gpio_dt_spec status_led;
int peripherals_init(void);
void application_timer_start(void);
/* Native nrfx transaction helpers for the one physical bus and UART. */
nrfx_err_t twim_transfer(uint8_t address, uint8_t *bytes, uint32_t size, bool read);
nrfx_err_t uarte_transmit(const uint8_t *bytes, uint32_t size, uint32_t *transmitted);
#endif
