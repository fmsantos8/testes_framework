#include "gpio.h"
void MX_GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    GPIO_InitTypeDef pin = {0};
    pin.Pin = LED_Pin;
    pin.Mode = GPIO_MODE_OUTPUT_PP;
    pin.Pull = GPIO_NOPULL;
    pin.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_Port, &pin);
    pin.Pin = BUTTON_Pin;
    pin.Mode = GPIO_MODE_INPUT;
    pin.Pull = GPIO_NOPULL; /* External 10 kohm pull-up. */
    HAL_GPIO_Init(BUTTON_GPIO_Port, &pin);
}
