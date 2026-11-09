/**
 * @file cmp_led_types.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Public LED context, configuration, and result types.
 */
#ifndef CMP_LED_TYPES_H
#define CMP_LED_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Caller-owned context, defined in cmp_led_config.h. */
typedef struct led led_t;

/** @brief Construction dependencies, defined in cmp_led_config.h. */
typedef struct led_config led_config_t;

/** @brief Component results, independent of HAL and SDK status types. */
typedef enum {
    LED_STATUS_OK = 0,           /** Operation completed successfully. */
    LED_STATUS_INVALID_ARGUMENT, /** A required pointer is NULL. */
    LED_STATUS_NOT_INITIALIZED,  /** Context or GPIO is unavailable. */
    LED_STATUS_NOT_SUPPORTED,    /** GPIO cannot be written as an output. */
    LED_STATUS_GPIO_ERROR        /** GPIO write failed. */
} led_status_t;

#ifdef __cplusplus
}
#endif

#endif
