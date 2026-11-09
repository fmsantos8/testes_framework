/**
 * @file cmp_ssd1306_fonts_private.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Internal packed font representation.
 */
#ifndef CMP_SSD1306_FONTS_PRIVATE_H
#define CMP_SSD1306_FONTS_PRIVATE_H

#include "cmp_ssd1306.h"

/** @brief One glyph, encoded as MSB-first rows with a byte-aligned row stride. */
typedef struct {
    uint32_t codepoint; /** Supported Unicode codepoint. */
    uint16_t offset;    /** Byte offset into the font bitmap. */
    uint8_t width;      /** Bitmap width in pixels. */
} ssd1306_glyph_t;

/** @brief Immutable font data; each glyph uses the font's fixed height. */
typedef struct {
    const uint8_t *bitmap;         /** Packed pixels. */
    const ssd1306_glyph_t *glyphs; /** Available glyphs. */
    uint32_t count;                /** Number of entries in glyphs. */
    uint32_t height;               /** Text box height in pixels. */
} ssd1306_font_data_t;

/** @brief Return immutable font data, or NULL for an unsupported selection. */
const ssd1306_font_data_t *cmp_ssd1306_font_get(ssd1306_font_t font);

#endif
