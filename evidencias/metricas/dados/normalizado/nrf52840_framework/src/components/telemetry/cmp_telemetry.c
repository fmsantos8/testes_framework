/**
 * @file cmp_telemetry.c
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Bounded integer record formatting and synchronous UART transmission.
 */
#include "cmp_telemetry_config.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>

static telemetry_status_t cmp_telemetry_translate_status(hal_status_t status) {
    switch (status) {
    case HAL_STATUS_OK:
        return TELEMETRY_STATUS_OK;
    case HAL_STATUS_INVALID_ARGUMENT:
        return TELEMETRY_STATUS_INVALID_ARGUMENT;
    case HAL_STATUS_NOT_INITIALIZED:
        return TELEMETRY_STATUS_NOT_INITIALIZED;
    case HAL_STATUS_NOT_SUPPORTED:
        return TELEMETRY_STATUS_NOT_SUPPORTED;
    case HAL_STATUS_TIMEOUT:
        return TELEMETRY_STATUS_TIMEOUT;
    default:
        return TELEMETRY_STATUS_COMMUNICATION_ERROR;
    }
}

static telemetry_status_t cmp_telemetry_require_initialized(const telemetry_t *telemetry, uint32_t *transmitted_count) {
    if (transmitted_count != NULL) {
        *transmitted_count = 0;
    }
    if (telemetry == NULL || transmitted_count == NULL) {
        return TELEMETRY_STATUS_INVALID_ARGUMENT;
    }
    return telemetry->initialized ? TELEMETRY_STATUS_OK : TELEMETRY_STATUS_NOT_INITIALIZED;
}

static bool cmp_telemetry_valid_source(const char *source) {
    if (source == NULL || source[0] == '\0') {
        return false;
    }
    for (uint32_t i = 0; i <= TELEMETRY_SOURCE_MAX_LENGTH; ++i) {
        unsigned char ch = (unsigned char)source[i];
        if (ch == '\0') {
            return true;
        }
        if (i == TELEMETRY_SOURCE_MAX_LENGTH || !((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
                                                  (ch >= '0' && ch <= '9') || ch == '_' || ch == '-' || ch == '.')) {
            return false;
        }
    }
    return false;
}

static telemetry_status_t cmp_telemetry_send_record(const telemetry_t *telemetry, const char *record, int length,
                                                    uint32_t capacity, uint32_t *transmitted_count) {
    if (length < 0 || (uint32_t)length >= capacity) {
        return TELEMETRY_STATUS_FORMAT_ERROR;
    }
    return cmp_telemetry_translate_status(
        hal_uart_write(telemetry->uart, (const uint8_t *)record, (uint32_t)length, transmitted_count));
}

telemetry_status_t cmp_telemetry_init(telemetry_t *telemetry, const telemetry_config_t *config) {
    if (telemetry == NULL || config == NULL || config->uart == NULL) {
        return TELEMETRY_STATUS_INVALID_ARGUMENT;
    }
    *telemetry = (telemetry_t){.uart = config->uart};

    uint32_t transmitted;
    telemetry_status_t status = cmp_telemetry_translate_status(hal_uart_write(telemetry->uart, NULL, 0, &transmitted));
    if (status == TELEMETRY_STATUS_OK) {
        telemetry->initialized = true;
    }
    return status;
}

telemetry_status_t cmp_telemetry_send_sample(const telemetry_t *telemetry, const telemetry_sample_t *sample,
                                             uint32_t *transmitted_count) {
    telemetry_status_t status = cmp_telemetry_require_initialized(telemetry, transmitted_count);
    if (sample == NULL) {
        return TELEMETRY_STATUS_INVALID_ARGUMENT;
    }
    if (status != TELEMETRY_STATUS_OK) {
        return status;
    }

    char record[128] = {0};
    int length = snprintf(record, sizeof(record), "SAMPLE;temperature_mC=%" PRId32 ";humidity_mpercent=%" PRIu32 "\r\n",
                          sample->temperature, sample->humidity);
    return cmp_telemetry_send_record(telemetry, record, length, (uint32_t)sizeof(record), transmitted_count);
}

telemetry_status_t cmp_telemetry_send_error(const telemetry_t *telemetry, const char *source, int32_t code,
                                            uint32_t *transmitted_count) {
    telemetry_status_t status = cmp_telemetry_require_initialized(telemetry, transmitted_count);
    if (!cmp_telemetry_valid_source(source)) {
        return TELEMETRY_STATUS_INVALID_ARGUMENT;
    }
    if (status != TELEMETRY_STATUS_OK) {
        return status;
    }

    char record[TELEMETRY_SOURCE_MAX_LENGTH + 64] = {0};
    int32_t length = snprintf(record, sizeof(record), "ERROR;source=%s;code=%" PRId32 "\r\n", source, code);
    return cmp_telemetry_send_record(telemetry, record, length, (uint32_t)sizeof(record), transmitted_count);
}

telemetry_status_t cmp_telemetry_resync(const telemetry_t *telemetry, uint32_t *transmitted_count) {
    telemetry_status_t status = cmp_telemetry_require_initialized(telemetry, transmitted_count);
    if (status != TELEMETRY_STATUS_OK) {
        return status;
    }

    const uint8_t delimiter[] = {'\r', '\n'};
    return cmp_telemetry_translate_status(
        hal_uart_write(telemetry->uart, delimiter, (uint32_t)sizeof(delimiter), transmitted_count));
}
