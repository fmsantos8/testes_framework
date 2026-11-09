#include "main.h"
void HAL_MspInit(void) {
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
}
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c) {
    if (hi2c->Instance != I2C1)
        return;
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef pin = {0};
    pin.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    pin.Mode = GPIO_MODE_AF_OD;
    pin.Pull = GPIO_NOPULL;
    pin.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    pin.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &pin);
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_I2C1_FORCE_RESET();
    __HAL_RCC_I2C1_RELEASE_RESET();
}
void HAL_UART_MspInit(UART_HandleTypeDef *huart) {
    if (huart->Instance != USART2)
        return;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_USART2_FORCE_RESET();
    __HAL_RCC_USART2_RELEASE_RESET();
    GPIO_InitTypeDef pin = {0};
    pin.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    pin.Mode = GPIO_MODE_AF_PP;
    pin.Pull = GPIO_NOPULL;
    pin.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    pin.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &pin);
}
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim) {
    if (htim->Instance != TIM2)
        return;
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_TIM2_FORCE_RESET();
    __HAL_RCC_TIM2_RELEASE_RESET();
    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}
