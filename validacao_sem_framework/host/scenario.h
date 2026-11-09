#ifndef SCENARIO_H
#define SCENARIO_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
enum { IO_OK, IO_ERROR, IO_TIMEOUT };
extern volatile uint32_t app_ticks;
extern uint32_t step;
int bus_write(uint8_t address, const uint8_t *data, uint32_t size);
int bus_read(uint8_t address, uint8_t *data, uint32_t size);
int uart_write(const uint8_t *data, uint32_t size, uint32_t *accepted);
bool button_pressed(void);
void led_write(bool value);
void firmware_init(void);
void firmware_process(void);
#endif
