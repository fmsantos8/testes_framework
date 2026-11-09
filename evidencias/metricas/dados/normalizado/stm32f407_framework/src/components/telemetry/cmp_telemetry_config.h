/**
 * @file cmp_telemetry_config.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Construction and caller-owned storage for the telemetry component.
 */
#ifndef CMP_TELEMETRY_CONFIG_H
#define CMP_TELEMETRY_CONFIG_H

#include <stdbool.h>

#include "cmp_telemetry.h"
#include "uart/hal_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief UART resource configuration. */
struct telemetry_config {
    hal_uart_t *uart; /** Required, already constructed UART with a finite timeout. */
};

/**
 * @brief Caller-owned persistent context; fields are private to the component.
 *
 * @details cmp_telemetry_init() sets all fields. No singleton or HAL arena allocation.
 */
struct telemetry {
    hal_uart_t *uart; /** UART handle. */
    bool initialized; /** True after successful UART verification. */
};

#ifdef __cplusplus
}
#endif

#endif
