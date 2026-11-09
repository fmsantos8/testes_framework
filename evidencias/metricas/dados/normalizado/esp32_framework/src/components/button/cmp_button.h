/**
 * @file cmp_button.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Logical button readings with board-supplied polarity.
 *
 */
#ifndef CMP_BUTTON_H
#define CMP_BUTTON_H

#include <stdbool.h>
#include <stdint.h>

#include "cmp_button_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize all context fields and verify that the GPIO can be read.
 *
 * @param[out] button Required caller-owned context.
 * @param[in] config Required input GPIO, time function, active level, and debounce interval.
 *
 * @retval BUTTON_STATUS_OK Context initialized and GPIO readable.
 * @retval BUTTON_STATUS_INVALID_ARGUMENT A required pointer or time function is NULL.
 * @retval BUTTON_STATUS_NOT_INITIALIZED GPIO is unavailable.
 * @retval BUTTON_STATUS_NOT_SUPPORTED GPIO cannot be read as an input.
 * @retval BUTTON_STATUS_GPIO_ERROR GPIO read failed.
 */
button_status_t cmp_button_init(button_t *button, const button_config_t *config);

/**
 * @brief Sample the GPIO and return its debounced logical state.
 *
 * @param[in] button Required initialized context.
 * @param[out] pressed Required output; true means pressed, false means released.
 *
 * @retval BUTTON_STATUS_OK Logical state written.
 * @retval BUTTON_STATUS_INVALID_ARGUMENT A required pointer is NULL.
 * @retval BUTTON_STATUS_NOT_INITIALIZED Context or GPIO is unavailable.
 * @retval BUTTON_STATUS_NOT_SUPPORTED GPIO cannot be read as an input.
 * @retval BUTTON_STATUS_GPIO_ERROR GPIO read failed.
 */
button_status_t cmp_button_read(button_t *button, bool *pressed);

#ifdef __cplusplus
}
#endif

#endif
