/**
 * @file cmp_ssd1306_types.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Public SSD1306 context, status, font, and bitmap types.
 */
#ifndef CMP_SSD1306_TYPES_H
#define CMP_SSD1306_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/** @brief Caller-owned context; its storage definition is in cmp_ssd1306_config.h. */
typedef struct ssd1306 ssd1306_t;

/** @brief Construction dependencies, defined in cmp_ssd1306_config.h. */
typedef struct ssd1306_config ssd1306_config_t;

/** @brief Component results, independent of HAL and SDK status types. */
typedef enum {
    SSD1306_STATUS_OK = 0,             /** Operation completed successfully. */
    SSD1306_STATUS_INVALID_ARGUMENT,   /** A required pointer, coordinate, size, or address is invalid. */
    SSD1306_STATUS_NOT_INITIALIZED,    /** The display or its I2C dependency is not initialized. */
    SSD1306_STATUS_NOT_SUPPORTED,      /** The selected font, character, or HAL operation is unavailable. */
    SSD1306_STATUS_TIMEOUT,            /** The configured I2C driver timeout expired. */
    SSD1306_STATUS_COMMUNICATION_ERROR /** The I2C dependency could not complete a transfer. */
} ssd1306_status_t;

/** @brief Built-in monochrome fonts; unsupported characters are rejected. */
typedef enum {
    SSD1306_FONT_SMALL = 0, /** 5x7 pixels, printable ASCII and the UTF-8 degree symbol. */
    SSD1306_FONT_MEDIUM,    /** Proportional font, 16 pixels high, printable ASCII and the UTF-8 degree symbol. */
    SSD1306_FONT_LARGE      /** Proportional numerals, 28 pixels high: space, digits, '+', '-', '.', ','. */
} ssd1306_font_t;

/**
 * @brief Monochrome image, stored as rows of ceil(width / 8) bytes, MSB first.
 *
 * @details Zero bits clear pixels and one bits set pixels. Unused low bits in
 * each row's final byte are ignored. The caller retains ownership of data.
 */
typedef struct {
    const uint8_t *data; /** Readable source bytes, independent of the display context. */
    uint32_t size;       /** Available byte count; must cover every row. */
    uint32_t width;      /** Positive image width in pixels. */
    uint32_t height;     /** Positive image height in pixels. */
} ssd1306_bitmap_t;

#ifdef __cplusplus
}
#endif

#endif
