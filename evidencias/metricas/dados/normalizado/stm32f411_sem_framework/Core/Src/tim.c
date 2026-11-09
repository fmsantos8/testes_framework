#include "tim.h"
TIM_HandleTypeDef htim2;
void MX_TIM2_Init(void) {
    uint32_t timer_clock = HAL_RCC_GetPCLK1Freq() * 2U; /* Both boards divide APB1. */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = timer_clock / 1000000U - 1U;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 999;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
        Error_Handler();
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
    HAL_NVIC_ClearPendingIRQ(TIM2_IRQn);
}
