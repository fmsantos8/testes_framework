/**
 * @file cmp_ssd1306.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Framebuffer graphics and incremental updates for a 128x64 I2C SSD1306.
 *
 * Serialize calls and shared-bus access in normal context; no ISR or
 * synchronization support. Context and bus must outlive their uses; inputs and
 * outputs must not overlap context storage. No dynamic allocation is used.
 * Except cmp_ssd1306_init(), calls require a previously initialized or zeroed context.
 * Drawing changes RAM only, validates bounds before editing, and never clips.
 * Coordinates start at the top left. Storage/configuration: cmp_ssd1306_config.h.
 */
#ifndef CMP_SSD1306_H
#define CMP_SSD1306_H

#include <stdbool.h>
#include <stdint.h>

#include "cmp_ssd1306_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SSD1306_WIDTH 128 /** Panel width in pixels. */
#define SSD1306_HEIGHT 64 /** Panel height in pixels. */

#define SSD1306_FRAMEBUFFER_SIZE (SSD1306_WIDTH * SSD1306_HEIGHT / 8) /** Framebuffer size (bytes). */

/**
 * @brief Configure the controller, clear its RAM while off, and turn it on.
 *
 * @details Initializes all context fields; no prior zeroing required. Invalid
 * arguments preserve the context. A transfer failure leaves it unavailable and
 * may partly configure the panel. Retry cmp_ssd1306_init() to recover; existing drawing is lost.
 * Transfers are synchronous and use the supplied bus timeout.
 *
 * @param[out] display Required caller-owned context.
 * @param[in] config Required bus handle and address (0x3C or 0x3D).
 *
 * @retval SSD1306_STATUS_OK Display initialized and blank.
 * @retval SSD1306_STATUS_INVALID_ARGUMENT Invalid pointer or address.
 * @retval SSD1306_STATUS_NOT_INITIALIZED I2C bus is unavailable.
 * @retval SSD1306_STATUS_NOT_SUPPORTED Required I2C operation is unavailable.
 * @retval SSD1306_STATUS_TIMEOUT I2C driver timeout expired.
 * @retval SSD1306_STATUS_COMMUNICATION_ERROR I2C transfer failed.
 */
ssd1306_status_t cmp_ssd1306_init(ssd1306_t *display, const ssd1306_config_t *config);

/**
 * @brief Clear the framebuffer and mark changed pages for update.
 *
 * @param[in,out] display Required initialized context.
 *
 * @retval SSD1306_STATUS_OK Framebuffer cleared.
 * @retval SSD1306_STATUS_INVALID_ARGUMENT display is NULL.
 * @retval SSD1306_STATUS_NOT_INITIALIZED Context is unavailable.
 */
ssd1306_status_t cmp_ssd1306_clear(ssd1306_t *display);

/**
 * @brief Set or clear a nonempty rectangle entirely inside the panel.
 *
 * @param[in,out] display Required initialized context.
 * @param[in] x Left column in pixels.
 * @param[in] y Top row in pixels.
 * @param[in] width Positive width in pixels.
 * @param[in] height Positive height in pixels.
 * @param[in] on true sets pixels; false clears them.
 *
 * @retval SSD1306_STATUS_OK Rectangle drawn.
 * @retval SSD1306_STATUS_INVALID_ARGUMENT NULL context, zero size, or invalid bounds.
 * @retval SSD1306_STATUS_NOT_INITIALIZED Context is unavailable.
 */
ssd1306_status_t cmp_ssd1306_fill_rect(ssd1306_t *display, uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                                       bool on);

/**
 * @brief Set or clear a line, including both endpoints, inside the panel.
 *
 * @param[in,out] display Required initialized context.
 * @param[in] x0 Start column in pixels.
 * @param[in] y0 Start row in pixels.
 * @param[in] x1 End column in pixels.
 * @param[in] y1 End row in pixels.
 * @param[in] on true sets pixels; false clears them.
 *
 * @retval SSD1306_STATUS_OK Line drawn.
 * @retval SSD1306_STATUS_INVALID_ARGUMENT NULL context or endpoint outside the panel.
 * @retval SSD1306_STATUS_NOT_INITIALIZED Context is unavailable.
 */
ssd1306_status_t cmp_ssd1306_draw_line(ssd1306_t *display, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, bool on);

/**
 * @brief Draw white text on a transparent background.
 *
 * @details Supports each font's character set, without wrapping or newlines.
 * The entire font-height box must fit, even for empty text. Use cmp_ssd1306_fill_rect() to
 * erase an old value before drawing its replacement.
 *
 * @param[in,out] display Required initialized context.
 * @param[in] x Left column in pixels.
 * @param[in] y Top row in pixels.
 * @param[in] text Required NUL-terminated text; may be empty.
 * @param[in] font Built-in font selection.
 *
 * @retval SSD1306_STATUS_OK Text drawn, or empty text accepted.
 * @retval SSD1306_STATUS_INVALID_ARGUMENT Invalid pointer, bounds, or width overflow.
 * @retval SSD1306_STATUS_NOT_INITIALIZED Context is unavailable.
 * @retval SSD1306_STATUS_NOT_SUPPORTED Font or character is unavailable.
 */
ssd1306_status_t cmp_ssd1306_draw_text(ssd1306_t *display, uint32_t x, uint32_t y, const char *text,
                                       ssd1306_font_t font);

/**
 * @brief Measure the text box used by cmp_ssd1306_draw_text(), without a display.
 *
 * @details Width includes one pixel between glyphs, with no trailing gap. Empty
 * text has zero width and the font's fixed height. Outputs change only on success.
 *
 * @param[in] text Required NUL-terminated text; may be empty.
 * @param[in] font Built-in font selection.
 * @param[out] width Required width output in pixels.
 * @param[out] height Required height output in pixels; must differ from width.
 *
 * @retval SSD1306_STATUS_OK Dimensions written.
 * @retval SSD1306_STATUS_INVALID_ARGUMENT NULL pointer, aliased outputs, or width overflow.
 * @retval SSD1306_STATUS_NOT_SUPPORTED Font or character is unavailable.
 */
ssd1306_status_t cmp_ssd1306_measure_text(const char *text, ssd1306_font_t font, uint32_t *width, uint32_t *height);

/**
 * @brief Copy an opaque bitmap entirely inside the panel into the framebuffer.
 *
 * @param[in,out] display Required initialized context.
 * @param[in] x Left column in pixels.
 * @param[in] y Top row in pixels.
 * @param[in] bitmap Required nonempty image with sufficient source bytes.
 *
 * @retval SSD1306_STATUS_OK Bitmap copied.
 * @retval SSD1306_STATUS_INVALID_ARGUMENT Invalid pointer, bitmap, or bounds.
 * @retval SSD1306_STATUS_NOT_INITIALIZED Context is unavailable.
 */
ssd1306_status_t cmp_ssd1306_draw_bitmap(ssd1306_t *display, uint32_t x, uint32_t y, const ssd1306_bitmap_t *bitmap);

/**
 * @brief Transfer at most one dirty 128x8 page and report pending work.
 *
 * @details Uses two synchronous I2C writes per dirty page (commands and 129 control/
 * data bytes); no traffic if clean. Pages appear progressively, without an atomic
 * frame swap or strict whole-call deadline. On failure, the page stays dirty and
 * the output is unchanged; retry resends both writes. Reinit/redraw after a reset.
 *
 * @param[in,out] display Required initialized context.
 * @param[out] pending Required output; true if dirty pages remain after success.
 *
 * @retval SSD1306_STATUS_OK Page sent, or no pending work.
 * @retval SSD1306_STATUS_INVALID_ARGUMENT Invalid pointer or I2C argument.
 * @retval SSD1306_STATUS_NOT_INITIALIZED Context or I2C bus is unavailable.
 * @retval SSD1306_STATUS_NOT_SUPPORTED Required I2C operation is unavailable.
 * @retval SSD1306_STATUS_TIMEOUT I2C driver timeout expired.
 * @retval SSD1306_STATUS_COMMUNICATION_ERROR I2C transfer failed.
 */
ssd1306_status_t cmp_ssd1306_update(ssd1306_t *display, bool *pending);

#ifdef __cplusplus
}
#endif

#endif
