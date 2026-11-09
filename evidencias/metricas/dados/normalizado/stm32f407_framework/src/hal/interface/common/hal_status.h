/**
 * @file hal_status.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Result codes for HAL operations.
 *
 */
#ifndef HAL_STATUS_H
#define HAL_STATUS_H

/**
 * @brief Result of a framework operation, independent of the platform SDK.
 */
typedef enum {
    HAL_STATUS_OK = 0,               /** The operation completed successfully */
    HAL_STATUS_ERROR = 1,            /** A platform or communication operation failed */
    HAL_STATUS_TIMEOUT = 2,          /** The configured platform transfer-wait mechanism timed out */
    HAL_STATUS_INVALID_ARGUMENT = 3, /** A pointer, identifier, value, or combination is invalid */
    HAL_STATUS_NOT_SUPPORTED = 4,    /** A valid operation or configuration is unavailable */
    HAL_STATUS_NOT_INITIALIZED = 5,  /** Required platform, memory, or resource initialization is missing */
    HAL_STATUS_NO_MEMORY = 6,        /** Required object or platform-driver storage is unavailable */
    HAL_STATUS_RESOURCE_BUSY = 7,    /** The requested hardware resource is already assigned */
    HAL_STATUS_INVALID_STATE = 8     /** The operation is forbidden in the current lifecycle state */
} hal_status_t;

#endif
