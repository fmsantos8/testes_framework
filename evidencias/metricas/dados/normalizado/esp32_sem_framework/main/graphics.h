#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdbool.h>
#include <stdint.h>
#define SSD1306_WIDTH 128U
#define SSD1306_HEIGHT 64U
#define SSD1306_FRAMEBUFFER_SIZE 1024U
typedef enum {
    SSD1306_STATUS_OK = 0,             /** Operation completed successfully. */
    SSD1306_STATUS_INVALID_ARGUMENT,   /** A required pointer, coordinate, size, or address is invalid. */
    SSD1306_STATUS_NOT_INITIALIZED,    /** The display or its I2C dependency is not initialized. */
    SSD1306_STATUS_NOT_SUPPORTED,      /** The selected font, character, or I2C operation is unavailable. */
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

typedef struct {
    uint8_t framebuffer[SSD1306_FRAMEBUFFER_SIZE];
    uint8_t dirty_pages;
} ssd1306_t;
ssd1306_status_t graphics_clear(ssd1306_t *display);
ssd1306_status_t graphics_fill_rect(ssd1306_t *display, uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                                    bool on);
ssd1306_status_t graphics_draw_line(ssd1306_t *display, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, bool on);
ssd1306_status_t graphics_draw_text(ssd1306_t *display, uint32_t x, uint32_t y, const char *text, ssd1306_font_t font);
ssd1306_status_t graphics_measure_text(const char *text, ssd1306_font_t font, uint32_t *width, uint32_t *height);
ssd1306_status_t graphics_draw_bitmap(ssd1306_t *display, uint32_t x, uint32_t y, const ssd1306_bitmap_t *bitmap);
#endif
