/**
 * @file cmp_telemetry_types.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Public telemetry context, configuration, sample, and result types.
 */
#ifndef CMP_TELEMETRY_TYPES_H
#define CMP_TELEMETRY_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Caller-owned context, defined in cmp_telemetry_config.h. */
typedef struct telemetry telemetry_t;

/** @brief Construction dependencies, defined in cmp_telemetry_config.h. */
typedef struct telemetry_config telemetry_config_t;

/** @brief Component results, independent of HAL and SDK status types. */
typedef enum {
    TELEMETRY_STATUS_OK = 0,              /** Complete record transmitted. */
    TELEMETRY_STATUS_INVALID_ARGUMENT,    /** A required pointer or source identifier is invalid. */
    TELEMETRY_STATUS_NOT_INITIALIZED,     /** Context or UART is unavailable. */
    TELEMETRY_STATUS_NOT_SUPPORTED,       /** Required UART operation is unavailable. */
    TELEMETRY_STATUS_TIMEOUT,             /** Configured UART driver timeout expired. */
    TELEMETRY_STATUS_COMMUNICATION_ERROR, /** UART could not complete the transfer. */
    TELEMETRY_STATUS_FORMAT_ERROR         /** Record could not be formatted within the internal buffer. */
} telemetry_status_t;

/** @brief Reading supplied by the caller; no sensor-specific dependency. */
typedef struct {
    int32_t temperature; /** Temperature in 0.001 degrees Celsius. */
    uint32_t humidity;   /** Relative humidity in 0.001 percent RH. */
} telemetry_sample_t;

#ifdef __cplusplus
}
#endif

#endif
