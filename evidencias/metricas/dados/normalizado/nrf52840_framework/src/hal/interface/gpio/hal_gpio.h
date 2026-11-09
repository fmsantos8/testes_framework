/**
 * @file hal_gpio.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Portable digital input and push-pull output contract.
 *
 */
#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdbool.h>

#include "common/hal_pin.h"
#include "common/hal_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque GPIO object owned by the framework.
 */
typedef struct hal_gpio hal_gpio_t;

/**
 * @brief Digital direction selected when constructing a GPIO object.
 */
typedef enum {
    HAL_GPIO_INPUT = 0, /** Digital input sampled through hal_gpio_read() */
    HAL_GPIO_OUTPUT     /** Push-pull digital output controlled through hal_gpio_write() */
} hal_gpio_direction_t;

/**
 * @brief Internal resistor selection; external resistors are unaffected.
 */
typedef enum {
    HAL_GPIO_PULL_NONE = 0, /** Disable internal pull-up and pull-down resistors */
    HAL_GPIO_PULL_UP,       /** Enable an internal pull-up resistor */
    HAL_GPIO_PULL_DOWN      /** Enable an internal pull-down resistor */
} hal_gpio_pull_t;

/**
 * @brief GPIO configuration.
 *
 * @details GPIO operations always use the physical electrical level.
 *
 */
typedef struct {
    hal_pin_t pin;                  /** Physical bank and pin */
    hal_gpio_direction_t direction; /** Digital input or push-pull output */
    hal_gpio_pull_t pull;           /** Requested internal resistor configuration */
    bool initial_level;             /** Initial output level: false is low, true is high; ignored for inputs */
} hal_gpio_config_t;

/**
 * @brief Construct a GPIO object and configure its physical pin during startup.
 *
 * @details The port validates all fields and pin capabilities before allocation.
 * For an output, the requested initial level is established as part of enabling
 * the output; a global guarantee of glitch-free transitions is not provided.
 * Construction requires an initialized, unsealed framework memory.
 * The framework owns object storage; failed construction may consume arena
 * space without guaranteeing memory or hardware rollback.
 *
 * @param[out] gpio GPIO object pointer to receive the allocated handle; must not be NULL.
 * @param[in] config Pointer to the GPIO configuration. must not be NULL
 *
 * @retval HAL_STATUS_OK The object and pin are ready for the configured direction.
 * @retval HAL_STATUS_INVALID_ARGUMENT A pointer, pin identifier, or enum value is invalid.
 * @retval HAL_STATUS_NOT_SUPPORTED The selected pin cannot provide the requested configuration.
 * @retval HAL_STATUS_NOT_INITIALIZED Platform or arena initialization is missing.
 * @retval HAL_STATUS_INVALID_STATE The construction phase has ended.
 * @retval HAL_STATUS_RESOURCE_BUSY The physical pin is already assigned.
 * @retval HAL_STATUS_NO_MEMORY Object storage is unavailable.
 * @retval HAL_STATUS_ERROR The platform failed to configure the pin.
 */
hal_status_t hal_gpio_config(hal_gpio_t **gpio, const hal_gpio_config_t *config);

/**
 * @brief Synchronously sample the physical level of a digital input.
 *
 * @param[in] gpio Valid GPIO handle configured as an input.
 * @param[out] level Receives false for low or true for high, only on success.
 *
 * @retval HAL_STATUS_OK The input level was read.
 * @retval HAL_STATUS_INVALID_ARGUMENT A required pointer is NULL.
 * @retval HAL_STATUS_NOT_INITIALIZED The resource is not ready.
 * @retval HAL_STATUS_NOT_SUPPORTED The handle is configured as an output.
 * @retval HAL_STATUS_ERROR The platform could not read the pin.
 */
hal_status_t hal_gpio_read(hal_gpio_t *gpio, bool *level);

/**
 * @brief Synchronously set the physical level of a push-pull output.
 *
 * @param[in] gpio Valid GPIO handle configured as an output.
 * @param[in] level Requested electrical level: false is low and true is high.
 *
 * @retval HAL_STATUS_OK The output operation was issued successfully.
 * @retval HAL_STATUS_INVALID_ARGUMENT The handle is NULL.
 * @retval HAL_STATUS_NOT_INITIALIZED The resource is not ready.
 * @retval HAL_STATUS_NOT_SUPPORTED The handle is configured as an input.
 * @retval HAL_STATUS_ERROR The platform could not write the pin.
 */
hal_status_t hal_gpio_write(hal_gpio_t *gpio, bool level);

#ifdef __cplusplus
}
#endif

#endif
