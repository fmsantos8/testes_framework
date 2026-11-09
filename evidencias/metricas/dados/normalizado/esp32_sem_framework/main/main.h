#ifndef MAIN_H
#define MAIN_H
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#define BUTTON_PIN GPIO_NUM_18
#define LED_PIN GPIO_NUM_19
extern i2c_master_dev_handle_t sensor_device;
extern i2c_master_dev_handle_t display_device;
#endif
