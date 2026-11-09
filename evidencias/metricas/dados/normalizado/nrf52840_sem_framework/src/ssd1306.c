#include "ssd1306.h"
#include "main.h"
#include <string.h>
ssd1306_t oled;
static bool initialized;
static ssd1306_status_t write_bytes(const uint8_t *bytes, uint32_t size) {
    nrfx_err_t status = twim_transfer(0x3C, (uint8_t *)bytes, size, false);
    return status == NRFX_SUCCESS         ? SSD1306_STATUS_OK
           : status == NRFX_ERROR_TIMEOUT ? SSD1306_STATUS_TIMEOUT
                                          : SSD1306_STATUS_COMMUNICATION_ERROR;
}
static ssd1306_status_t write_page(unsigned page) {
    const uint8_t commands[] = {0x00, (uint8_t)(0xB0U | page), 0x00, 0x10};
    ssd1306_status_t status = write_bytes(commands, sizeof(commands));
    if (status != SSD1306_STATUS_OK)
        return status;
    uint8_t packet[129];
    packet[0] = 0x40;
    memcpy(packet + 1, oled.framebuffer + page * 128U, 128U);
    return write_bytes(packet, sizeof(packet));
}
ssd1306_status_t ssd1306_init(void) {
    initialized = false;
    memset(&oled, 0, sizeof(oled));
    const uint8_t commands[] = {0x00, 0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0x8D,
                                0x14, 0x20, 0x02, 0xA1, 0xC8, 0xDA, 0x12, 0x81, 0xCF, 0xD9,
                                0xF1, 0xDB, 0x30, 0x2E, 0x23, 0x00, 0xD6, 0x00, 0xA4, 0xA6};
    ssd1306_status_t status = write_bytes(commands, sizeof(commands));
    if (status != SSD1306_STATUS_OK)
        return status;
    for (unsigned page = 0; page < 8; ++page) {
        status = write_page(page);
        if (status != SSD1306_STATUS_OK)
            return status;
    }
    const uint8_t on[] = {0x00, 0xAF};
    status = write_bytes(on, sizeof(on));
    initialized = status == SSD1306_STATUS_OK;
    return status;
}
ssd1306_status_t ssd1306_update(bool *pending) {
    if (pending == NULL)
        return SSD1306_STATUS_INVALID_ARGUMENT;
    if (!initialized)
        return SSD1306_STATUS_NOT_INITIALIZED;
    for (unsigned page = 0; page < 8; ++page) {
        uint8_t mask = (uint8_t)(1U << page);
        if (oled.dirty_pages & mask) {
            ssd1306_status_t status = write_page(page);
            if (status != SSD1306_STATUS_OK)
                return status;
            oled.dirty_pages &= (uint8_t)~mask;
            break;
        }
    }
    *pending = oled.dirty_pages != 0;
    return SSD1306_STATUS_OK;
}
