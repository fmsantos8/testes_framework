#ifndef SSD1306_H
#define SSD1306_H
#include "graphics.h"
extern ssd1306_t oled;
ssd1306_status_t ssd1306_init(void);
ssd1306_status_t ssd1306_update(bool *pending);
#endif
