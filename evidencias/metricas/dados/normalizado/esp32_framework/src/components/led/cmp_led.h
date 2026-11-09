/**
 * @file cmp_led.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Logical LED control with board-supplied polarity.
 *
 */
#ifndef CMP_LED_H
#define CMP_LED_H

#include <stdbool.h>

#include "cmp_led_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize all context fields and switch the LED off.
 *
 * @param[out] led Required caller-owned context.
 * @param[in] config Required constructed output GPIO and active level.
 *
 * @retval LED_STATUS_OK Context initialized and off command issued successfully.
 * @retval LED_STATUS_INVALID_ARGUMENT A required pointer is NULL.
 * @retval LED_STATUS_NOT_INITIALIZED GPIO is unavailable.
 * @retval LED_STATUS_NOT_SUPPORTED GPIO cannot be written as an output.
 * @retval LED_STATUS_GPIO_ERROR GPIO write failed.
 */
led_status_t cmp_led_init(led_t *led, const led_config_t *config);

/**
 * @brief Switch the LED on or off, applying its configured polarity.
 *
 * @param[in] led Required initialized context.
 * @param[in] on true requests illumination; false requests the LED off.
 *
 * @retval LED_STATUS_OK Output command issued successfully.
 * @retval LED_STATUS_INVALID_ARGUMENT led is NULL.
 * @retval LED_STATUS_NOT_INITIALIZED Context or GPIO is unavailable.
 * @retval LED_STATUS_NOT_SUPPORTED GPIO cannot be written as an output.
 * @retval LED_STATUS_GPIO_ERROR GPIO write failed.
 */
led_status_t cmp_led_set(const led_t *led, bool on);

#ifdef __cplusplus
}
#endif

#endif
