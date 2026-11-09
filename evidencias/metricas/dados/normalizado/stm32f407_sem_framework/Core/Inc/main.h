#ifndef MAIN_H
#define MAIN_H
#include "stm32f4xx_hal.h"
#define LED_Pin GPIO_PIN_1
#define LED_GPIO_Port GPIOA
#define BUTTON_Pin GPIO_PIN_0
#define BUTTON_GPIO_Port GPIOA
void Error_Handler(void);
void SystemClock_Config(void);
#endif
