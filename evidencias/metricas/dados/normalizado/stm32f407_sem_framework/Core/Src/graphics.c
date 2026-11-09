/* Pixel algorithms derived from the reference SSD1306 implementation. */
#include "graphics.h"
#include "fonts.h"
#include <stddef.h>
#define PAGE_HEIGHT 8U
static bool graphics_rectangle_fits(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    return x < SSD1306_WIDTH && y < SSD1306_HEIGHT && width <= SSD1306_WIDTH - x && height <= SSD1306_HEIGHT - y;
}

static void graphics_put_pixel(ssd1306_t *display, uint32_t x, uint32_t y, bool on) {
    uint32_t page = y / PAGE_HEIGHT;
    uint32_t index = page * SSD1306_WIDTH + x;
    uint8_t mask = (uint8_t)(1U << (y % PAGE_HEIGHT));
    uint8_t previous = display->framebuffer[index];
    uint8_t value = on ? (uint8_t)(previous | mask) : (uint8_t)(previous & (uint8_t)~mask);
    if (value != previous) {
        display->framebuffer[index] = value;
        display->dirty_pages |= (uint8_t)(1U << page);
    }
}

static ssd1306_status_t graphics_next_codepoint(const uint8_t **cursor, uint32_t *codepoint) {
    if (**cursor >= 0x20 && **cursor <= 0x7E) {
        *codepoint = **cursor;
        ++*cursor;
        return SSD1306_STATUS_OK;
    }
    if ((*cursor)[0] == 0xC2 && (*cursor)[1] == 0xB0) {
        *codepoint = 0xB0;
        *cursor += 2;
        return SSD1306_STATUS_OK;
    }
    return SSD1306_STATUS_NOT_SUPPORTED;
}

static const ssd1306_glyph_t *graphics_find_glyph(const ssd1306_font_data_t *font, uint32_t codepoint) {
    for (uint32_t i = 0; i < font->count; ++i) {
        if (font->glyphs[i].codepoint == codepoint) {
            return &font->glyphs[i];
        }
    }
    return NULL;
}

static ssd1306_status_t graphics_measure_text_width(const char *text, const ssd1306_font_data_t *font,
                                                    uint32_t *width) {
    const uint8_t *cursor = (const uint8_t *)text;
    uint32_t measured_width = 0;
    while (*cursor != 0) {
        uint32_t codepoint;
        ssd1306_status_t status = graphics_next_codepoint(&cursor, &codepoint);
        if (status != SSD1306_STATUS_OK) {
            return status;
        }
        const ssd1306_glyph_t *glyph = graphics_find_glyph(font, codepoint);
        if (glyph == NULL) {
            return SSD1306_STATUS_NOT_SUPPORTED;
        }
        uint32_t advance = glyph->width + (measured_width == 0 ? 0U : 1U);
        if (measured_width > UINT32_MAX - advance) {
            return SSD1306_STATUS_INVALID_ARGUMENT;
        }
        measured_width += advance;
    }
    *width = measured_width;
    return SSD1306_STATUS_OK;
}

ssd1306_status_t graphics_clear(ssd1306_t *display) {
    ssd1306_status_t status = display == NULL ? SSD1306_STATUS_INVALID_ARGUMENT : SSD1306_STATUS_OK;
    if (status != SSD1306_STATUS_OK) {
        return status;
    }
    for (uint32_t i = 0; i < SSD1306_FRAMEBUFFER_SIZE; ++i) {
        if (display->framebuffer[i] != 0) {
            display->framebuffer[i] = 0;
            display->dirty_pages |= (uint8_t)(1U << (i / SSD1306_WIDTH));
        }
    }
    return SSD1306_STATUS_OK;
}

ssd1306_status_t graphics_fill_rect(ssd1306_t *display, uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                                    bool on) {
    ssd1306_status_t status = display == NULL ? SSD1306_STATUS_INVALID_ARGUMENT : SSD1306_STATUS_OK;
    if (status != SSD1306_STATUS_OK) {
        return status;
    }
    if (width == 0 || height == 0 || !graphics_rectangle_fits(x, y, width, height)) {
        return SSD1306_STATUS_INVALID_ARGUMENT;
    }
    for (uint32_t row = 0; row < height; ++row) {
        for (uint32_t column = 0; column < width; ++column) {
            graphics_put_pixel(display, x + column, y + row, on);
        }
    }
    return SSD1306_STATUS_OK;
}

ssd1306_status_t graphics_draw_line(ssd1306_t *display, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, bool on) {
    ssd1306_status_t status = display == NULL ? SSD1306_STATUS_INVALID_ARGUMENT : SSD1306_STATUS_OK;
    if (status != SSD1306_STATUS_OK) {
        return status;
    }
    if (x0 >= SSD1306_WIDTH || x1 >= SSD1306_WIDTH || y0 >= SSD1306_HEIGHT || y1 >= SSD1306_HEIGHT) {
        return SSD1306_STATUS_INVALID_ARGUMENT;
    }

    int32_t x = (int32_t)x0;
    int32_t y = (int32_t)y0;
    int32_t end_x = (int32_t)x1;
    int32_t end_y = (int32_t)y1;
    int32_t dx = x < end_x ? end_x - x : x - end_x;
    int32_t dy = y < end_y ? y - end_y : end_y - y;
    int32_t step_x = x < end_x ? 1 : -1;
    int32_t step_y = y < end_y ? 1 : -1;
    int32_t error = dx + dy;
    while (true) {
        graphics_put_pixel(display, (uint32_t)x, (uint32_t)y, on);
        if (x == end_x && y == end_y) {
            break;
        }
        int32_t doubled = 2 * error;
        if (doubled >= dy) {
            error += dy;
            x += step_x;
        }
        if (doubled <= dx) {
            error += dx;
            y += step_y;
        }
    }
    return SSD1306_STATUS_OK;
}

ssd1306_status_t graphics_draw_text(ssd1306_t *display, uint32_t x, uint32_t y, const char *text, ssd1306_font_t font) {
    ssd1306_status_t status = display == NULL ? SSD1306_STATUS_INVALID_ARGUMENT : SSD1306_STATUS_OK;
    if (status != SSD1306_STATUS_OK) {
        return status;
    }
    if (text == NULL) {
        return SSD1306_STATUS_INVALID_ARGUMENT;
    }
    const ssd1306_font_data_t *data = graphics_font_get(font);
    if (data == NULL) {
        return SSD1306_STATUS_NOT_SUPPORTED;
    }
    uint32_t width;
    status = graphics_measure_text_width(text, data, &width);
    if (status != SSD1306_STATUS_OK) {
        return status;
    }
    if (!graphics_rectangle_fits(x, y, width, data->height)) {
        return SSD1306_STATUS_INVALID_ARGUMENT;
    }

    // Validation above ensures the entire text can be drawn without partial edits.
    const uint8_t *cursor = (const uint8_t *)text;
    while (*cursor != 0) {
        uint32_t codepoint;
        status = graphics_next_codepoint(&cursor, &codepoint);
        if (status != SSD1306_STATUS_OK) {
            return status;
        }
        const ssd1306_glyph_t *glyph = graphics_find_glyph(data, codepoint);
        if (glyph == NULL) {
            return SSD1306_STATUS_NOT_SUPPORTED;
        }
        uint32_t stride = (glyph->width + 7U) / 8U;
        for (uint32_t row = 0; row < data->height; ++row) {
            for (uint32_t column = 0; column < glyph->width; ++column) {
                uint8_t pixels = data->bitmap[glyph->offset + row * stride + column / 8U];
                if ((pixels & (0x80U >> (column % 8U))) != 0) {
                    graphics_put_pixel(display, x + column, y + row, true);
                }
            }
        }
        x += glyph->width + 1U;
    }
    return SSD1306_STATUS_OK;
}

ssd1306_status_t graphics_measure_text(const char *text, ssd1306_font_t font, uint32_t *width, uint32_t *height) {
    if (text == NULL || width == NULL || height == NULL || width == height) {
        return SSD1306_STATUS_INVALID_ARGUMENT;
    }
    const ssd1306_font_data_t *data = graphics_font_get(font);
    if (data == NULL) {
        return SSD1306_STATUS_NOT_SUPPORTED;
    }
    uint32_t measured_width;
    ssd1306_status_t status = graphics_measure_text_width(text, data, &measured_width);
    if (status == SSD1306_STATUS_OK) {
        *width = measured_width;
        *height = data->height;
    }
    return status;
}

ssd1306_status_t graphics_draw_bitmap(ssd1306_t *display, uint32_t x, uint32_t y, const ssd1306_bitmap_t *bitmap) {
    ssd1306_status_t status = display == NULL ? SSD1306_STATUS_INVALID_ARGUMENT : SSD1306_STATUS_OK;
    if (status != SSD1306_STATUS_OK) {
        return status;
    }
    if (bitmap == NULL || bitmap->data == NULL || bitmap->width == 0 || bitmap->height == 0 ||
        !graphics_rectangle_fits(x, y, bitmap->width, bitmap->height)) {
        return SSD1306_STATUS_INVALID_ARGUMENT;
    }
    uint32_t stride = (bitmap->width + 7U) / 8U;
    if (bitmap->size < stride * bitmap->height) {
        return SSD1306_STATUS_INVALID_ARGUMENT;
    }
    for (uint32_t row = 0; row < bitmap->height; ++row) {
        for (uint32_t column = 0; column < bitmap->width; ++column) {
            uint8_t pixels = bitmap->data[row * stride + column / 8U];
            graphics_put_pixel(display, x + column, y + row, (pixels & (0x80U >> (column % 8U))) != 0);
        }
    }
    return SSD1306_STATUS_OK;
}
