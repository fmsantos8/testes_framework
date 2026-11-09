/**
 * @file cmp_button_config.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Construction and caller-owned storage for the button component.
 */
#ifndef CMP_BUTTON_CONFIG_H
#define CMP_BUTTON_CONFIG_H

#include "cmp_button.h"

#include "gpio/hal_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CMP_BUTTON_DEFAULT_DEBOUNCE 30U /** Default stable sample interval (ms) */

/**
 * @brief Input resource and electrical polarity supplied by the composition.
 *
 * @details The BSP configures the input and its pull resistors. This component
 * uses the input GPIO without selecting pins or constructing GPIO resources.
 */
struct button_config {
    hal_gpio_t *gpio;             /** Required, already constructed input GPIO. */
    bool active_level;            /** Electrical level representing a pressed button. */
    uint32_t debounce;            /** Stable sample interval in milliseconds; zero selects the 30 ms default. */
    button_get_ticks_t get_ticks; /** Required time source supplied by the integration. */
};

/**
 * @brief Caller-owned persistent context; fields are private to the component.
 *
 * @details Declare in the composition root and pass button_t pointers to
 * consumers. cmp_button_init() sets all fields. No singleton or HAL arena allocation.
 */
struct button {
    hal_gpio_t *gpio;             /** Input GPIO handle. */
    bool active_level;            /** Electrical level representing a pressed button. */
    bool initialized;             /** True after successful GPIO verification. */
    uint32_t debounce;            /** Effective stable sample interval in milliseconds. */
    uint32_t candidate_since;     /** Timestamp in milliseconds of the first sample of the candidate state. */
    bool candidate_pressed;       /** Most recently sampled logical state. */
    bool candidate_valid;         /** False before the first sample or after a GPIO failure. */
    bool stable_pressed;          /** Last accepted state; initially released. */
    button_get_ticks_t get_ticks; /** Function to get time source in milliseconds */
};

#ifdef __cplusplus
}
#endif

#endif
