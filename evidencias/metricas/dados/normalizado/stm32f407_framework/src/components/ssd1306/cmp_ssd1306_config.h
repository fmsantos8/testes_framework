/**
 * @file cmp_ssd1306_config.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Construction and static storage for the SSD1306 composition root.
 */
#ifndef CMP_SSD1306_CONFIG_H
#define CMP_SSD1306_CONFIG_H

#include "cmp_ssd1306.h"

#include "i2c/hal_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Dependencies for a 128x64 module with an internal charge pump.
 *
 * @details The board supplies stable power, a completed hardware reset, and I2C
 * pull-ups. The composition supplies a constructed bus and the device address;
 * this component does not select pins, configure the bus, or own its handle.
 */
struct ssd1306_config {
    hal_i2c_t *i2c;  /** Required, already constructed I2C master bus. */
    uint8_t address; /** Unshifted SSD1306 address: 0x3C or 0x3D. */
};

/**
 * @brief Caller-owned persistent storage; fields are private to the component.
 *
 * @details Declare this in the composition root and pass ssd1306_t pointers to
 * consumers. No singleton or HAL arena allocation is used. A static instance is
 * recommended; cmp_ssd1306_init() supplies all initial values, even for automatic storage.
 */
struct ssd1306 {
    hal_i2c_t *i2c;                                /** I2C bus handle. */
    uint8_t address;                               /** Device address. */
    uint8_t framebuffer[SSD1306_FRAMEBUFFER_SIZE]; /** Page-major pixels, bit zero at the top of each page. */
    uint8_t dirty_pages;                           /** One bit per page requiring transfer. */
    bool initialized;                              /** True only after a complete successful initialization. */
};

#ifdef __cplusplus
}
#endif

#endif
