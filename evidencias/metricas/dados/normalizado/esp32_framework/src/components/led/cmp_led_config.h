/**
 * @file cmp_led_config.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Construction and caller-owned storage for the LED component.
 */
#ifndef CMP_LED_CONFIG_H
#define CMP_LED_CONFIG_H

#include "cmp_led.h"

#include "gpio/hal_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Output resource and electrical polarity supplied by the composition.
 *
 * @details The BSP configures the output and its initial level. This component
 * uses the output GPIO without selecting pins or constructing GPIO resources.
 * The caller reserves output control for this component.
 */
struct led_config {
    hal_gpio_t *gpio;  /** Required, already constructed output GPIO. */
    bool active_level; /** Electrical level that illuminates the LED. */
};

/**
 * @brief Caller-owned persistent context; fields are private to the component.
 *
 * @details Declare in the composition root and pass led_t pointers to consumers.
 * cmp_led_init() sets all fields. No singleton or HAL arena allocation.
 */
struct led {
    hal_gpio_t *gpio;  /** Output GPIO handle. */
    bool active_level; /** Electrical level that illuminates the LED. */
    bool initialized;  /** True after the initial off command succeeds. */
};

#ifdef __cplusplus
}
#endif

#endif
