#include "scenario.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
volatile uint32_t app_ticks;
uint32_t step;
static int last_led = -1;
static unsigned uart_attempt;
static unsigned scenario;
static void print_bytes(const uint8_t *bytes, uint32_t size) {
    for (uint32_t i = 0; i < size; ++i) printf("%02x", bytes[i]);
    putchar('\n');
}
static uint8_t crc(const uint8_t *data) {
    uint8_t value = 0xFF;
    for (unsigned i = 0; i < 2; ++i) {
        value ^= data[i];
        for (unsigned b = 0; b < 8; ++b) value = value & 128 ? (uint8_t)((value << 1) ^ 0x31) : (uint8_t)(value << 1);
    }
    return value;
}
int bus_write(uint8_t address, const uint8_t *data, uint32_t size) {
    int status = IO_OK;
    if (step >= 2000 && step < 3000 && address == 0x44) status = IO_TIMEOUT;
    if (step >= 6100 && step < 7300 && address == 0x3C && size == 129) status = IO_ERROR;
    if (scenario == 2 && step >= 18000 && step < 28000 && address == 0x3C) status = IO_ERROR;
    if (step >= 12000 && step < 13000 && address == 0x44) status = IO_ERROR;
    printf("W %u %02x %u %d ", app_ticks, address, size, status);
    print_bytes(data, size);
    return status;
}
int bus_read(uint8_t address, uint8_t *data, uint32_t size) {
    if (address != 0x44 || size != 6) abort();
    int status = step >= 14000 && step < 15000 ? IO_TIMEOUT : IO_OK;
    uint16_t temperature = step < 6000 ? 0x6666 : step < 8000 ? 0 : step < 10000 ? 65535 : 0x8000;
    uint16_t humidity = step < 8000 ? 0x8000 : 65535;
    data[0] = temperature >> 8; data[1] = temperature & 255; data[2] = crc(data);
    data[3] = humidity >> 8; data[4] = humidity & 255; data[5] = crc(data + 3);
    if (step >= 4000 && step < 5000) data[2] ^= 1;
    printf("R %u %d ", app_ticks, status);
    print_bytes(data, 6);
    return status;
}
int uart_write(const uint8_t *data, uint32_t size, uint32_t *accepted) {
    ++uart_attempt;
    int status = IO_OK;
    *accepted = size;
    /* Exercise zero-prefix retry, partial discard, delimiter retry, full-prefix timeout. */
    if (uart_attempt == 2) { status = IO_TIMEOUT; *accepted = 0; }
    if (uart_attempt == 4) { status = IO_ERROR; *accepted = size > 5 ? 5 : size; }
    if (uart_attempt == 5) { status = IO_ERROR; *accepted = 0; }
    if (uart_attempt == 9) { status = IO_TIMEOUT; *accepted = size; }
    if (scenario == 2 && step >= 18000 && step < 35000) { status = IO_ERROR; *accepted = 0; }
    printf("U %u %d %u ", app_ticks, status, *accepted);
    for (uint32_t i = 0; i < *accepted; ++i) printf("%02x",data[i]);
    putchar('\n');
    return status;
}
bool button_pressed(void) {
    /* Bounce, sustained press, release bounce, and a second sustained press. */
    return (step >= 500 && step < 510 && step % 2) || (step >= 510 && step < 600) ||
           (step >= 600 && step < 609 && step % 2) || (step >= 3400 && step < 3500) ||
           (step >= 6050 && step < 6150);
}
void led_write(bool value) {
    if (last_led != (int)value) { printf("L %u %d\n", app_ticks, value); last_led = value; }
}
int main(int argc, char **argv) {
    scenario = argc > 1 ? (unsigned)strtoul(argv[1],NULL,10) : 0;
    app_ticks = scenario == 1 ? UINT32_MAX - 1000U : 0;
    firmware_init();
    for (step = 0; step < (scenario == 2 ? 40000U : 18000U); ++step) { firmware_process(); ++app_ticks; }
    return 0;
}
