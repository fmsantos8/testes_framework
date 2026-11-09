/**
 * @file hal_system.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief System initialization
 */
#ifndef HAL_SYSTEM_H
#define HAL_SYSTEM_H

#include <stdint.h>

#include "common/hal_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief System startup requirements and HAL storage supplied by the integration.
 *
 * @details cpu_clock selects the nominal CPU frequency, not an oscillator,
 * peripheral bus or timer frequency. The BSP or integration chooses this value;
 * the port implements the supported clock configuration. Board-dependent clock
 * inputs, such as an external crystal frequency, remain in the build configuration.
 */
typedef struct {
    void *memory_buffer;      /** Persistent writable storage aligned for max_align_t. */
    uint32_t memory_capacity; /** Positive buffer size in bytes; includes allocation padding. */
    uint32_t cpu_clock;       /** Required nominal CPU frequency in hertz; must be positive. */
} hal_system_config_t;

/**
 * @brief Initialize the platform infrastructure and open HAL construction.
 *
 * @details Call once at startup, before HAL construction and after any SDK
 * startup required by the port. Validate the configuration and supported CPU
 * frequency before allocation or hardware setup. The port configures the requested
 * nominal CPU frequency or verifies a matching frequency already established by
 * the SDK. Fixed-frequency or build-configured platforms may support only that
 * existing frequency. Ports document supported values and SDK prerequisites;
 * unavailable requests return NOT_SUPPORTED without rounding or substitution.
 *
 * Oscillator, PLL, bus dividers and Flash latency are handled by the port using
 * the board/SDK build configuration, respecting prior SDK initialization. Resource
 * constructors complete peripheral clocks, routing, and configuration after this
 * call succeeds. The nominal CPU frequency remains fixed during normal operation;
 * runtime clock reconfiguration and dynamic frequency scaling are outside this
 * contract. The integration must preserve this configuration while resources are
 * in use.
 *
 * @param[in] config Required system configuration.
 *
 * @retval HAL_STATUS_OK The requested nominal CPU frequency is established or
 * verified, and the system is ready for HAL resource construction.
 * @retval HAL_STATUS_INVALID_ARGUMENT config or memory_buffer is NULL, capacity
 * or cpu_clock is zero, or the buffer alignment is invalid.
 * @retval HAL_STATUS_NOT_SUPPORTED The requested CPU frequency cannot be provided
 * by the hardware or the selected SDK/build configuration.
 * @retval HAL_STATUS_INVALID_STATE System initialization has already succeeded.
 * @retval HAL_STATUS_ERROR Required platform initialization or establishment or
 * verification of a supported CPU frequency failed.
 */
hal_status_t hal_system_init(const hal_system_config_t *config);

/**
 * @brief Permanently close HAL object allocation after resource construction.
 *
 * @retval HAL_STATUS_OK Construction is closed, including a repeated seal.
 * @retval HAL_STATUS_NOT_INITIALIZED hal_system_init() has not succeeded.
 */
hal_status_t hal_system_seal(void);

#ifdef __cplusplus
}
#endif

#endif
