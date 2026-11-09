/**
 * @file cmp_button.c
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief GPIO-backed logical button readings.
 */
#include "cmp_button_config.h"

#include <stddef.h>

static button_status_t cmp_button_translate_status(hal_status_t status) {
    switch (status) {
    case HAL_STATUS_OK:
        return BUTTON_STATUS_OK;
    case HAL_STATUS_INVALID_ARGUMENT:
        return BUTTON_STATUS_INVALID_ARGUMENT;
    case HAL_STATUS_NOT_INITIALIZED:
        return BUTTON_STATUS_NOT_INITIALIZED;
    case HAL_STATUS_NOT_SUPPORTED:
        return BUTTON_STATUS_NOT_SUPPORTED;
    default:
        return BUTTON_STATUS_GPIO_ERROR;
    }
}

button_status_t cmp_button_init(button_t *button, const button_config_t *config) {
    if (button == NULL || config == NULL || config->gpio == NULL || config->get_ticks == NULL) {
        return BUTTON_STATUS_INVALID_ARGUMENT;
    }
    *button = (button_t){
        .gpio = config->gpio,
        .active_level = config->active_level,
        .debounce = config->debounce != 0 ? config->debounce : CMP_BUTTON_DEFAULT_DEBOUNCE,
        .get_ticks = config->get_ticks,
    };

    // Verify the input GPIO without retaining or publishing its initial state.
    bool level;
    button_status_t status = cmp_button_translate_status(hal_gpio_read(button->gpio, &level));
    if (status == BUTTON_STATUS_OK) {
        button->initialized = true;
    }
    return status;
}

button_status_t cmp_button_read(button_t *button, bool *pressed) {
    if (button == NULL || pressed == NULL) {
        return BUTTON_STATUS_INVALID_ARGUMENT;
    }
    if (!button->initialized) {
        return BUTTON_STATUS_NOT_INITIALIZED;
    }

    bool level;
    button_status_t status = cmp_button_translate_status(hal_gpio_read(button->gpio, &level));
    if (status != BUTTON_STATUS_OK) {
        button->candidate_valid = false;
        return status;
    }

    uint32_t now = button->get_ticks();
    bool sample_pressed = level == button->active_level;
    if (!button->candidate_valid || sample_pressed != button->candidate_pressed) {
        button->candidate_pressed = sample_pressed;
        button->candidate_since = now;
        button->candidate_valid = true;
    } else if ((uint32_t)(now - button->candidate_since) >= button->debounce) {
        button->stable_pressed = sample_pressed;
    }
    *pressed = button->stable_pressed;
    return BUTTON_STATUS_OK;
}
