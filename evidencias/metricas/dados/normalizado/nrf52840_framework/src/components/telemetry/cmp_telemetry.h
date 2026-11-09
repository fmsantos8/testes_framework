/**
 * @file cmp_telemetry.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Common ASCII records for readings and application-selected errors.
 *
 */
#ifndef CMP_TELEMETRY_H
#define CMP_TELEMETRY_H

#include "cmp_telemetry_types.h"

#define TELEMETRY_SOURCE_MAX_LENGTH 32 /** Maximum source identifier length, excluding NUL. */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize all fields and verify the UART with a zero-byte write.
 *
 * @details No prior zeroing required. Invalid arguments preserve the context.
 * A valid attempt replaces it; failure leaves it unavailable. Initialization
 * can be retried and emits no bytes. Does not construct or reconfigure the UART.
 *
 * @param[out] telemetry Required caller-owned context.
 * @param[in] config Required constructed UART dependency.
 *
 * @retval TELEMETRY_STATUS_OK Context initialized and UART available.
 * @retval TELEMETRY_STATUS_INVALID_ARGUMENT A required pointer is NULL or UART rejects the request.
 * @retval TELEMETRY_STATUS_NOT_INITIALIZED UART is unavailable.
 * @retval TELEMETRY_STATUS_NOT_SUPPORTED UART operation is unavailable.
 * @retval TELEMETRY_STATUS_TIMEOUT UART driver timeout expired.
 * @retval TELEMETRY_STATUS_COMMUNICATION_ERROR UART verification failed.
 */
telemetry_status_t cmp_telemetry_init(telemetry_t *telemetry, const telemetry_config_t *config);

/**
 * @brief Send SAMPLE;temperature_mC=<signed>;humidity_mpercent=<unsigned>\r\n.
 *
 * @details Decimal integers cover the complete field ranges, independent of
 * locale and floating-point support. Physical range validation belongs to the caller.
 *
 * @param[in] telemetry Required initialized context.
 * @param[in] sample Required reading to format.
 * @param[out] transmitted_count Required accepted byte count, including any CRLF bytes.
 *
 * @retval TELEMETRY_STATUS_OK Complete record transmitted.
 * @retval TELEMETRY_STATUS_INVALID_ARGUMENT A required pointer is NULL or UART rejects the request.
 * @retval TELEMETRY_STATUS_NOT_INITIALIZED Context or UART is unavailable.
 * @retval TELEMETRY_STATUS_NOT_SUPPORTED UART operation is unavailable.
 * @retval TELEMETRY_STATUS_TIMEOUT UART timeout expired; count may be nonzero.
 * @retval TELEMETRY_STATUS_COMMUNICATION_ERROR UART transfer failed; count may be nonzero.
 * @retval TELEMETRY_STATUS_FORMAT_ERROR Formatting failed; no UART write occurred.
 */
telemetry_status_t cmp_telemetry_send_sample(const telemetry_t *telemetry, const telemetry_sample_t *sample,
                                             uint32_t *transmitted_count);

/**
 * @brief Send ERROR;source=<identifier>;code=<signed>\r\n.
 *
 * @param[in] telemetry Required initialized context.
 * @param[in] source Required source identifier, readable through its NUL or the first max+1 bytes.
 * @param[in] code Application-selected error code.
 * @param[out] transmitted_count Required accepted byte count, including any CRLF bytes.
 *
 * @retval TELEMETRY_STATUS_OK Complete record transmitted.
 * @retval TELEMETRY_STATUS_INVALID_ARGUMENT A pointer or source is invalid, or UART rejects the request.
 * @retval TELEMETRY_STATUS_NOT_INITIALIZED Context or UART is unavailable.
 * @retval TELEMETRY_STATUS_NOT_SUPPORTED UART operation is unavailable.
 * @retval TELEMETRY_STATUS_TIMEOUT UART timeout expired; count may be nonzero.
 * @retval TELEMETRY_STATUS_COMMUNICATION_ERROR UART transfer failed; count may be nonzero.
 * @retval TELEMETRY_STATUS_FORMAT_ERROR Formatting failed; no UART write occurred.
 */
telemetry_status_t cmp_telemetry_send_error(const telemetry_t *telemetry, const char *source, int32_t code,
                                            uint32_t *transmitted_count);

/**
 * @brief Send CRLF to terminate a truncated record before the next record.
 *
 * @details Attempts the complete two-byte delimiter on every call, without
 * retaining progress or retrying internally. The caller decides when to retry
 * and must wait for success before sending another record. Repeated attempts
 * may add CR bytes or empty lines; receivers must ignore incomplete or empty
 * lines. Uses the UART's configured timeout and does not reconfigure it.
 *
 * @param[in] telemetry Required initialized context.
 * @param[out] transmitted_count Required accepted byte count, zeroed before validation.
 *
 * @retval TELEMETRY_STATUS_OK Both delimiter bytes transmitted.
 * @retval TELEMETRY_STATUS_INVALID_ARGUMENT A required pointer is NULL or UART rejects the request.
 * @retval TELEMETRY_STATUS_NOT_INITIALIZED Context or UART is unavailable.
 * @retval TELEMETRY_STATUS_NOT_SUPPORTED UART operation is unavailable.
 * @retval TELEMETRY_STATUS_TIMEOUT UART timeout expired; count may be nonzero, including two.
 * @retval TELEMETRY_STATUS_COMMUNICATION_ERROR UART transfer failed; count may be nonzero, including two.
 */
telemetry_status_t cmp_telemetry_resync(const telemetry_t *telemetry, uint32_t *transmitted_count);

#ifdef __cplusplus
}
#endif

#endif
