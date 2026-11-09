/**
 * @file hal_i2c.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Synchronous, addressed I2C master bus contract.
 *
 */
#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stddef.h>
#include <stdint.h>

#include "common/hal_pin.h"
#include "common/hal_status.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "common/hal_status.h"

/**
 * @brief Opaque structure representing an I2C peripheral.
 *
 */
typedef struct hal_i2c hal_i2c_t;

/**
 * @brief I2C configuration structure.
 *
 * @details The board supplies external pull-ups. The port configures open-drain
 * routing; separate GPIO objects must not claim these pins. Build-time routing
 * may limit accepted configurations.
 * The constructor validates a positive, finite timeout before allocation or
 * hardware setup. Port-defined values/resolution apply; a fixed-limit port
 * accepts only its matching setting, otherwise returning NOT_SUPPORTED.
 * No infinite wait; the driver timeout is not a strict whole-call deadline.
 */
typedef struct {
    uint8_t instance;    /** Zero-based controller identifier mapped by the BSP */
    hal_pin_t clock_pin; /** Serial clock pin */
    hal_pin_t data_pin;  /** Serial data pin; must differ from clock_pin */
    uint32_t frequency;  /** Requested bus clock frequency in hertz; must be positive */
    uint32_t timeout;    /** Positive lifetime driver timeout in milliseconds; port-defined values/resolution */
} hal_i2c_config_t;

/**
 * @brief Construct a master bus object and configure its controller and pins.
 *
 * @param[out] i2c I2C object pointer to receive the allocated handle; must not be NULL.
 * @param[in] config Pointer to the GPIO configuration. must not be NULL
 *
 * @retval HAL_STATUS_OK The bus is ready; this does not probe any attached device.
 * @retval HAL_STATUS_INVALID_ARGUMENT A pointer, identifier, frequency, timeout, or pin combination is invalid.
 * @retval HAL_STATUS_NOT_SUPPORTED The requested frequency, routing, or timeout setting is unavailable.
 * @retval HAL_STATUS_NOT_INITIALIZED Platform or arena initialization is missing.
 * @retval HAL_STATUS_INVALID_STATE The construction phase has ended.
 * @retval HAL_STATUS_RESOURCE_BUSY The controller or either pin is already assigned.
 * @retval HAL_STATUS_NO_MEMORY Object storage is unavailable.
 * @retval HAL_STATUS_ERROR The platform failed to configure the bus.
 */
hal_status_t hal_i2c_config(hal_i2c_t **i2c, const hal_i2c_config_t *config);

/**
 * @brief Write a nonempty block in one synchronous transaction ending with STOP.
 *
 * @param[in] i2c I2C object pointer.
 * @param[in] address Unshifted seven-bit address from 0x08 through 0x77; reserved addresses are excluded.
 * @param[in] buffer Source data; must not be NULL.
 * @param[in] size Number of source bytes
 *
 * @retval HAL_STATUS_OK All bytes were accepted by the addressed device.
 * @retval HAL_STATUS_INVALID_ARGUMENT A pointer, address, or size is invalid.
 * @retval HAL_STATUS_NOT_INITIALIZED The bus is not ready.
 * @retval HAL_STATUS_NOT_SUPPORTED The port does not provide this operation or transfer size.
 * @retval HAL_STATUS_TIMEOUT The configured driver timeout expired; device state may have changed.
 * @retval HAL_STATUS_ERROR A NACK, arbitration loss, or other bus error occurred.
 */
hal_status_t hal_i2c_write(hal_i2c_t *i2c, uint8_t address, const uint8_t *buffer, uint32_t size);

/**
 * @brief Read a nonempty block in one synchronous transaction ending with STOP.
 *
 * @param[in] i2c I2C object pointer
 * @param[in] address Unshifted seven-bit address from 0x08 through 0x77.
 * @param[out] buffer Destination for size bytes; may contain partial data on failure.
 * @param[in] size Number of bytes to receive; must be positive.
 *
 * @retval HAL_STATUS_OK All requested bytes were received.
 * @retval HAL_STATUS_INVALID_ARGUMENT A pointer, address, or size is invalid.
 * @retval HAL_STATUS_NOT_INITIALIZED The bus is not ready.
 * @retval HAL_STATUS_NOT_SUPPORTED The port does not provide this operation or transfer size.
 * @retval HAL_STATUS_TIMEOUT The configured driver timeout expired; discard the destination contents.
 * @retval HAL_STATUS_ERROR A NACK, arbitration loss, or other bus error occurred.
 */
hal_status_t hal_i2c_read(hal_i2c_t *i2c, uint8_t address, uint8_t *buffer, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif
