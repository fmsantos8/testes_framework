/**
 * @file cmp_button_types.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Public button context, configuration, and result types.
 */
#ifndef CMP_BUTTON_TYPES_H
#define CMP_BUTTON_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Caller-owned context, defined in cmp_button_config.h. */
typedef struct button button_t;

/** @brief Construction dependencies, defined in cmp_button_config.h. */
typedef struct button_config button_config_t;

/**
 * @brief Return monotonic system ticks in milliseconds, modulo 2^32.
 * @details Supplied by the integration and called in normal execution context.
 * Must remain callable throughout the component lifetime and return promptly.
 * Synchronization with any interrupt-updated clock belongs to the provider.
 */
typedef uint32_t (*button_get_ticks_t)(void);

/** @brief Component results, independent of HAL and SDK status types. */
typedef enum {
    BUTTON_STATUS_OK = 0,           /** Operation completed successfully. */
    BUTTON_STATUS_INVALID_ARGUMENT, /** A required pointer is NULL. */
    BUTTON_STATUS_NOT_INITIALIZED,  /** Context or GPIO is unavailable. */
    BUTTON_STATUS_NOT_SUPPORTED,    /** GPIO cannot be read as an input. */
    BUTTON_STATUS_GPIO_ERROR        /** GPIO read failed. */
} button_status_t;

#ifdef __cplusplus
}
#endif

#endif
