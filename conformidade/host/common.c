/* One public-contract scenario set compiled against each unmodified port. */
#include "fixture.h"
#include "hal_memory.h"
#include "bsp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); exit(1); \
} } while (0)

fixture_mode_t fixture_bus_mode, fixture_uart_mode;
bool fixture_bus_init_failure;
uint8_t fixture_last_address;
uint32_t fixture_last_timeout;
hal_gpio_config_t fixture_input, fixture_output;
hal_i2c_config_t fixture_bus;
hal_uart_config_t fixture_serial;
hal_timer_config_t fixture_timer;
uint32_t fixture_cpu, fixture_i2c_limit;
static _Alignas(max_align_t) unsigned char storage[16384];
static uint32_t notifications;
static void notify(void) { ++notifications; }
static hal_system_config_t system_config(void) {
    return (hal_system_config_t){storage, sizeof(storage), fixture_cpu};
}
static void initialize(void) {
    hal_system_config_t config = system_config();
    CHECK(hal_system_init(&config) == HAL_STATUS_OK);
}
static uint32_t used(void) {
    hal_memory_info_t info;
    CHECK(hal_memory_get_info(&info) == HAL_MEMORY_OK);
    return info.used;
}

int main(int argc, char **argv) {
    CHECK(argc == 2);
    const char *id = argv[1];
    fixture_setup();
    fixture_timer.callback = notify;
    hal_gpio_t *gpio = NULL, *input = NULL;
    hal_i2c_t *bus = NULL;
    hal_uart_t *uart = NULL;
    hal_timer_t *timer = NULL;
    uint8_t bytes[6] = {1, 2, 3, 4, 5, 6};
    uint32_t count = 99;

    if (!strcmp(id, "BSP-01")) {
        bsp_resources_t resources = {0};
        CHECK(bsp_init(notify, &resources) == HAL_STATUS_OK);
        CHECK(resources.button && resources.led && resources.devices_i2c);
        CHECK(resources.telemetry_uart && resources.timebase_timer);
        CHECK(!resources.button_active_level && resources.led_active_level);
        CHECK(resources.sht31_address == 0x44 && resources.ssd1306_address == 0x3c);
        hal_memory_info_t info;
        CHECK(hal_memory_get_info(&info) == HAL_MEMORY_OK && info.sealed);
        CHECK(!fixture_output_level());
        fixture_fire_timer(); CHECK(notifications == 0);
        CHECK(hal_timer_start(resources.timebase_timer) == HAL_STATUS_OK);
        fixture_fire_timer(); CHECK(notifications == 1);
        CHECK(bsp_init(notify, &resources) == HAL_STATUS_INVALID_STATE);
    } else if (!strcmp(id, "SYS-01")) {
        CHECK(hal_system_seal() == HAL_STATUS_NOT_INITIALIZED);
        CHECK(hal_gpio_config(&gpio, &fixture_output) == HAL_STATUS_NOT_INITIALIZED && !gpio);
        CHECK(hal_i2c_config(&bus, &fixture_bus) == HAL_STATUS_NOT_INITIALIZED && !bus);
        CHECK(hal_uart_config(&uart, &fixture_serial) == HAL_STATUS_NOT_INITIALIZED && !uart);
        CHECK(hal_timer_config(&timer, &fixture_timer) == HAL_STATUS_NOT_INITIALIZED && !timer);
    } else if (!strcmp(id, "SYS-02")) {
        hal_system_config_t config = system_config();
        CHECK(hal_system_init(NULL) == HAL_STATUS_INVALID_ARGUMENT);
        config.memory_buffer = storage + 1;
        CHECK(hal_system_init(&config) == HAL_STATUS_INVALID_ARGUMENT);
        config = system_config(); config.memory_capacity = 0;
        CHECK(hal_system_init(&config) == HAL_STATUS_INVALID_ARGUMENT);
        config = system_config(); config.cpu_clock = 0;
        CHECK(hal_system_init(&config) == HAL_STATUS_INVALID_ARGUMENT);
        config = system_config(); config.cpu_clock = 1;
        CHECK(hal_system_init(&config) == HAL_STATUS_NOT_SUPPORTED);
        CHECK(hal_memory_get_info(&(hal_memory_info_t){0}) == HAL_MEMORY_NOT_INITIALIZED);
        initialize();
    } else if (!strcmp(id, "SYS-03")) {
        initialize();
        hal_system_config_t config = system_config();
        CHECK(hal_system_init(&config) == HAL_STATUS_INVALID_STATE);
        CHECK(hal_gpio_config(&gpio, &fixture_output) == HAL_STATUS_OK);
        CHECK(hal_i2c_config(&bus, &fixture_bus) == HAL_STATUS_OK);
        CHECK(hal_uart_config(&uart, &fixture_serial) == HAL_STATUS_OK);
        CHECK(hal_timer_config(&timer, &fixture_timer) == HAL_STATUS_OK);
        uint32_t before = used();
        CHECK(hal_system_seal() == HAL_STATUS_OK && hal_system_seal() == HAL_STATUS_OK);
        CHECK(hal_gpio_config(&input, &fixture_input) == HAL_STATUS_INVALID_STATE && !input);
        CHECK(hal_i2c_config(&bus, &fixture_bus) == HAL_STATUS_INVALID_STATE && !bus);
        CHECK(hal_uart_config(&uart, &fixture_serial) == HAL_STATUS_INVALID_STATE && !uart);
        hal_timer_t *other = NULL;
        CHECK(hal_timer_config(&other, &fixture_timer) == HAL_STATUS_INVALID_STATE && !other);
        CHECK(used() == before);
        CHECK(hal_gpio_write(gpio, true) == HAL_STATUS_OK);
        CHECK(hal_timer_start(timer) == HAL_STATUS_OK);
    } else if (!strcmp(id, "SYS-04")) {
        hal_system_config_t config = system_config(); config.memory_capacity = 1;
        CHECK(hal_system_init(&config) == HAL_STATUS_OK);
        CHECK(hal_gpio_config(&gpio, &fixture_output) == HAL_STATUS_NO_MEMORY && !gpio);
        CHECK(hal_i2c_config(&bus, &fixture_bus) == HAL_STATUS_NO_MEMORY && !bus);
        CHECK(hal_uart_config(&uart, &fixture_serial) == HAL_STATUS_NO_MEMORY && !uart);
        CHECK(hal_timer_config(&timer, &fixture_timer) == HAL_STATUS_NO_MEMORY && !timer);
        CHECK(used() == 0);
    } else if (!strcmp(id, "GPIO-01")) {
        initialize();
        CHECK(hal_gpio_config(&gpio, &fixture_output) == HAL_STATUS_OK);
        CHECK(!fixture_output_level());
        CHECK(hal_gpio_write(gpio, true) == HAL_STATUS_OK && fixture_output_level());
        CHECK(hal_gpio_config(&input, &fixture_input) == HAL_STATUS_OK);
        bool level = false;
        fixture_set_input(true);
        CHECK(hal_gpio_read(input, &level) == HAL_STATUS_OK && level);
        level = true;
        CHECK(hal_gpio_read(gpio, &level) == HAL_STATUS_NOT_SUPPORTED && level);
        CHECK(hal_gpio_write(input, true) == HAL_STATUS_NOT_SUPPORTED);
        CHECK(hal_gpio_read(NULL, &level) == HAL_STATUS_INVALID_ARGUMENT);
        CHECK(hal_gpio_read(input, NULL) == HAL_STATUS_INVALID_ARGUMENT);
        CHECK(hal_gpio_write(NULL, false) == HAL_STATUS_INVALID_ARGUMENT);
    } else if (!strcmp(id, "CFG-01")) {
        initialize();
        CHECK(hal_gpio_config(&gpio, NULL) == HAL_STATUS_INVALID_ARGUMENT && !gpio);
        CHECK(hal_i2c_config(&bus, NULL) == HAL_STATUS_INVALID_ARGUMENT && !bus);
        CHECK(hal_uart_config(&uart, NULL) == HAL_STATUS_INVALID_ARGUMENT && !uart);
        CHECK(hal_timer_config(&timer, NULL) == HAL_STATUS_INVALID_ARGUMENT && !timer);
        CHECK(hal_gpio_config(NULL, &fixture_output) == HAL_STATUS_INVALID_ARGUMENT);
        CHECK(hal_i2c_config(NULL, &fixture_bus) == HAL_STATUS_INVALID_ARGUMENT);
        CHECK(hal_uart_config(NULL, &fixture_serial) == HAL_STATUS_INVALID_ARGUMENT);
        CHECK(hal_timer_config(NULL, &fixture_timer) == HAL_STATUS_INVALID_ARGUMENT);
        fixture_output.direction = (hal_gpio_direction_t)99;
        fixture_bus.timeout = 0; fixture_serial.timeout = 0; fixture_timer.period = 0;
        CHECK(hal_gpio_config(&gpio, &fixture_output) == HAL_STATUS_INVALID_ARGUMENT);
        CHECK(hal_i2c_config(&bus, &fixture_bus) == HAL_STATUS_INVALID_ARGUMENT);
        CHECK(hal_uart_config(&uart, &fixture_serial) == HAL_STATUS_INVALID_ARGUMENT);
        CHECK(hal_timer_config(&timer, &fixture_timer) == HAL_STATUS_INVALID_ARGUMENT);
        CHECK(used() == 0);
    } else if (!strcmp(id, "CFG-02")) {
        initialize();
        fixture_bus.frequency = 1000000; fixture_serial.baudrate = 1;
        CHECK(hal_i2c_config(&bus, &fixture_bus) == HAL_STATUS_NOT_SUPPORTED && !bus);
        CHECK(hal_uart_config(&uart, &fixture_serial) == HAL_STATUS_NOT_SUPPORTED && !uart);
        CHECK(used() == 0);
    } else if (!strcmp(id, "CFG-03")) {
        initialize();
        fixture_output.pin.port = UINT8_MAX;
        fixture_bus.data_pin = fixture_bus.clock_pin;
        fixture_serial.parity = (hal_uart_parity_t)99;
        fixture_timer.instance = UINT8_MAX;
        CHECK(hal_gpio_config(&gpio, &fixture_output) == HAL_STATUS_INVALID_ARGUMENT);
        CHECK(hal_i2c_config(&bus, &fixture_bus) == HAL_STATUS_INVALID_ARGUMENT);
        CHECK(hal_uart_config(&uart, &fixture_serial) == HAL_STATUS_INVALID_ARGUMENT);
        CHECK(hal_timer_config(&timer, &fixture_timer) == HAL_STATUS_INVALID_ARGUMENT);
        CHECK(used() == 0);
    } else if (!strcmp(id, "RES-01")) {
        initialize();
        CHECK(hal_gpio_config(&gpio, &fixture_output) == HAL_STATUS_OK);
        CHECK(hal_i2c_config(&bus, &fixture_bus) == HAL_STATUS_OK);
        CHECK(hal_uart_config(&uart, &fixture_serial) == HAL_STATUS_OK);
        CHECK(hal_timer_config(&timer, &fixture_timer) == HAL_STATUS_OK);
        uint32_t before = used();
        CHECK(hal_gpio_config(&gpio, &fixture_output) == HAL_STATUS_RESOURCE_BUSY && !gpio);
        CHECK(hal_i2c_config(&bus, &fixture_bus) == HAL_STATUS_RESOURCE_BUSY && !bus);
        CHECK(hal_uart_config(&uart, &fixture_serial) == HAL_STATUS_RESOURCE_BUSY && !uart);
        CHECK(hal_timer_config(&timer, &fixture_timer) == HAL_STATUS_RESOURCE_BUSY && !timer);
        CHECK(used() == before);
    } else if (!strncmp(id, "I2C-", 4)) {
        initialize();
        if (!strcmp(id, "I2C-05")) {
            fixture_bus_init_failure = true;
            CHECK(hal_i2c_config(&bus, &fixture_bus) == HAL_STATUS_ERROR && !bus);
            CHECK(used() > 0); /* Failed SDK setup may consume arena storage. */
        } else {
            CHECK(hal_i2c_config(&bus, &fixture_bus) == HAL_STATUS_OK);
            if (!strcmp(id, "I2C-01")) {
                CHECK(hal_i2c_write(bus, 0x07, bytes, 1) == HAL_STATUS_INVALID_ARGUMENT);
                CHECK(hal_i2c_read(bus, 0x78, bytes, 1) == HAL_STATUS_INVALID_ARGUMENT);
                CHECK(hal_i2c_write(bus, 0x44, NULL, 1) == HAL_STATUS_INVALID_ARGUMENT);
                CHECK(hal_i2c_read(bus, 0x44, bytes, 0) == HAL_STATUS_INVALID_ARGUMENT);
                CHECK(hal_i2c_read(bus, 0x44, bytes, fixture_i2c_limit + 1) == HAL_STATUS_NOT_SUPPORTED);
            } else {
                if (!strcmp(id, "I2C-03")) fixture_bus_mode = F_ERROR;
                if (!strcmp(id, "I2C-04")) fixture_bus_mode = F_TIMEOUT;
                hal_status_t expected = fixture_bus_mode == F_OK ? HAL_STATUS_OK :
                    fixture_bus_mode == F_ERROR ? HAL_STATUS_ERROR : HAL_STATUS_TIMEOUT;
                CHECK(hal_i2c_write(bus, 0x44, bytes, 3) == expected);
                CHECK(hal_i2c_read(bus, 0x44, bytes, 6) == expected);
                if (expected == HAL_STATUS_OK) {
                    CHECK(fixture_last_address == 0x44 && fixture_last_timeout == fixture_bus.timeout);
                    CHECK(bytes[0] == 0x5a && bytes[5] == 0x5a);
                }
                fixture_bus_mode = F_OK;
                CHECK(hal_i2c_read(bus, 0x44, bytes, 6) == HAL_STATUS_OK);
            }
        }
    } else if (!strncmp(id, "UART-", 5)) {
        initialize();
        CHECK(hal_uart_config(&uart, &fixture_serial) == HAL_STATUS_OK);
        if (!strcmp(id, "UART-01")) {
            CHECK(hal_uart_write(NULL, bytes, 3, &count) == HAL_STATUS_INVALID_ARGUMENT && count == 0);
            CHECK(hal_uart_write(uart, NULL, 3, &count) == HAL_STATUS_INVALID_ARGUMENT && count == 0);
            CHECK(hal_uart_write(uart, bytes, 3, NULL) == HAL_STATUS_INVALID_ARGUMENT);
            count = 99;
            CHECK(hal_uart_write(uart, NULL, 0, &count) == HAL_STATUS_OK && count == 0);
        } else if (!strcmp(id, "UART-06")) {
            uint8_t block[129] = {0};
            CHECK(hal_uart_write(uart, block, sizeof(block), &count) == HAL_STATUS_OK);
            CHECK(count == sizeof(block));
        } else {
            if (!strcmp(id, "UART-03")) fixture_uart_mode = F_TIMEOUT;
            if (!strcmp(id, "UART-04")) fixture_uart_mode = F_PARTIAL;
            if (!strcmp(id, "UART-05")) fixture_uart_mode = F_ERROR;
            hal_status_t expected = fixture_uart_mode == F_OK ? HAL_STATUS_OK :
                fixture_uart_mode == F_ERROR ? HAL_STATUS_ERROR : HAL_STATUS_TIMEOUT;
            CHECK(hal_uart_write(uart, bytes, 3, &count) == expected);
            CHECK(count == (fixture_uart_mode == F_OK ? 3U : fixture_uart_mode == F_PARTIAL ? 2U : 0U));
        }
    } else if (!strncmp(id, "TIMER-", 6)) {
        initialize();
        if (!strcmp(id, "TIMER-01")) {
            CHECK(hal_timer_start(NULL) == HAL_STATUS_INVALID_ARGUMENT);
            fixture_timer.callback = NULL;
            CHECK(hal_timer_config(&timer, &fixture_timer) == HAL_STATUS_INVALID_ARGUMENT);
        } else if (!strcmp(id, "TIMER-03")) {
            if (!fixture_unsupported_timer(&fixture_timer)) { puts("NOT_APPLICABLE"); return 77; }
            CHECK(hal_timer_config(&timer, &fixture_timer) == HAL_STATUS_NOT_SUPPORTED && !timer);
            CHECK(used() == 0);
        } else {
            CHECK(hal_timer_config(&timer, &fixture_timer) == HAL_STATUS_OK);
            fixture_fire_timer(); CHECK(notifications == 0);
            CHECK(hal_system_seal() == HAL_STATUS_OK);
            CHECK(hal_timer_start(timer) == HAL_STATUS_OK);
            fixture_fire_timer(); CHECK(notifications == 1);
            CHECK(hal_timer_start(timer) == HAL_STATUS_INVALID_STATE);
            fixture_fire_timer(); CHECK(notifications == 2);
        }
    } else { fprintf(stderr, "Unknown case %s\n", id); return 2; }
    printf("PASS %s\n", id);
    return 0;
}
