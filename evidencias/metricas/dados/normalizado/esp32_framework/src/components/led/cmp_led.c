/**
 * @file cmp_led.c
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief GPIO-backed logical LED control.
 */
#include "cmp_led_config.h"

#include <stddef.h>

static led_status_t cmp_led_translate_status(hal_status_t status) {
    switch (status) {
    case HAL_STATUS_OK:
        return LED_STATUS_OK;
    case HAL_STATUS_INVALID_ARGUMENT:
        return LED_STATUS_INVALID_ARGUMENT;
    case HAL_STATUS_NOT_INITIALIZED:
        return LED_STATUS_NOT_INITIALIZED;
    case HAL_STATUS_NOT_SUPPORTED:
        return LED_STATUS_NOT_SUPPORTED;
    default:
        return LED_STATUS_GPIO_ERROR;
    }
}

led_status_t cmp_led_init(led_t *led, const led_config_t *config) {
    if (led == NULL || config == NULL || config->gpio == NULL) {
        return LED_STATUS_INVALID_ARGUMENT;
    }
    *led = (led_t){.gpio = config->gpio, .active_level = config->active_level};

    led_status_t status = cmp_led_translate_status(hal_gpio_write(led->gpio, !led->active_level));
    if (status == LED_STATUS_OK) {
        led->initialized = true;
    }
    return status;
}

led_status_t cmp_led_set(const led_t *led, bool on) {
    if (led == NULL) {
        return LED_STATUS_INVALID_ARGUMENT;
    }
    if (!led->initialized) {
        return LED_STATUS_NOT_INITIALIZED;
    }

    return cmp_led_translate_status(hal_gpio_write(led->gpio, on ? led->active_level : !led->active_level));
}
