/**
 * @file hal_timer.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Periodic notifications from a dedicated hardware timer.
 *
 */
#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "common/hal_status.h"

/**
 * @brief Opaque hardware timer object.
 */
typedef struct hal_timer hal_timer_t;

/**
 * @brief Handle a serviced hardware timer expiration in interrupt context.
 *
 * @details Keep callbacks short and nonblocking; call only ISR-safe HAL APIs.
 * Synchronize shared application data for the platform; volatile alone does
 * not ensure atomic access.
 */
typedef void (*hal_timer_callback_t)(void);

/**
 * @brief Fixed periodic configuration copied by the constructor.
 *
 * @details Reserves dedicated hardware for the firmware lifetime. The port
 * validates the period before allocation or setup and documents supported
 * periods/resolution. Unavailable settings return NOT_SUPPORTED without substitution.
 */
typedef struct {
    uint8_t instance;              /** Port-defined hardware timer resource identifier selected by the BSP */
    uint32_t period;               /** Positive interval between periodic hardware expirations, in milliseconds */
    hal_timer_callback_t callback; /** Required expiration callback; must not be NULL */
} hal_timer_config_t;

/**
 * @brief Construct and configure a periodic hardware timer without starting it.
 *
 * @details Construct sequentially with initialized, unsealed framework memory.
 * Configuration is fixed; callbacks begin only after hal_timer_start(). Failure
 * may consume arena space. No destruction or runtime reconfiguration.
 *
 * @param[out] timer Timer object pointer to receive the allocated handle; must not be NULL.
 * @param[in] config Complete startup configuration; must not be NULL.
 *
 * @retval HAL_STATUS_OK The timer is configured and stopped.
 * @retval HAL_STATUS_INVALID_ARGUMENT A required pointer, callback, identifier, or period is invalid.
 * @retval HAL_STATUS_NOT_SUPPORTED The requested period or dedicated hardware resource is unavailable on this platform.
 * @retval HAL_STATUS_NOT_INITIALIZED Platform or arena initialization is missing.
 * @retval HAL_STATUS_INVALID_STATE The construction phase has ended.
 * @retval HAL_STATUS_RESOURCE_BUSY The selected hardware resource is already assigned or the hardware timer pool is exhausted.
 * @retval HAL_STATUS_NO_MEMORY Framework or SDK storage is unavailable.
 * @retval HAL_STATUS_ERROR The platform failed to configure the timer.
 */
hal_status_t hal_timer_config(hal_timer_t **timer, const hal_timer_config_t *config);

/**
 * @brief Start a configured periodic timer once, after application initialization.
 *
 * @details Call in normal context after initializing dependencies and callback
 * data. Allocates no framework objects; allowed after arena sealing. First
 * expiration follows one period from hardware activation; the callback may run
 * before return. Repeated starts do not restart it; failed starts leave no callbacks.
 * Interrupt latency may delay callbacks or coalesce expirations under port-defined
 * conditions. Missed periods need not be recovered; a callback counter measures
 * delivered notifications, not necessarily physical elapsed time.
 *
 * @param[in] timer Valid handle returned by hal_timer_config().
 *
 * @retval HAL_STATUS_OK The timer is running with periodic notifications enabled.
 * @retval HAL_STATUS_INVALID_ARGUMENT The handle is NULL.
 * @retval HAL_STATUS_NOT_INITIALIZED The timer or platform is not ready.
 * @retval HAL_STATUS_INVALID_STATE The timer is already running.
 * @retval HAL_STATUS_ERROR The platform failed to start the timer.
 */
hal_status_t hal_timer_start(hal_timer_t *timer);

#ifdef __cplusplus
}
#endif

#endif
