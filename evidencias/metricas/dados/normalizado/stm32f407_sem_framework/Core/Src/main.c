#include "main.h"
#include "gpio.h"
#include "i2c.h"
#include "usart.h"
#include "tim.h"
#include "application.h"
#include "sht31d.h"
#include "ssd1306.h"
volatile uint32_t app_ticks;
int main(void) {
    SystemCoreClockUpdate();
    if (HAL_Init() != HAL_OK)
        Error_Handler();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART2_UART_Init();
    MX_TIM2_Init();
    if (sht31d_init() != SHT31D_STATUS_OK || ssd1306_init() != SSD1306_STATUS_OK)
        Error_Handler();
    application_init();
    if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
        Error_Handler();
    while (1)
        application_process();
}
void SystemClock_Config(void) {
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clocks = {0};
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
        Error_Handler();
    clocks.ClockType = RCC_CLOCKTYPE_SYSCLK;
    clocks.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    if (HAL_RCC_ClockConfig(&clocks, FLASH_LATENCY_5) != HAL_OK)
        Error_Handler();
    osc.PLL.PLLState = RCC_PLL_OFF;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
        Error_Handler();
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    osc.PLL.PLLM = 16;
    osc.PLL.PLLN = 336;
    osc.PLL.PLLP = RCC_PLLP_DIV2;
    osc.PLL.PLLQ = 7;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
        Error_Handler();
    uint32_t start = HAL_GetTick();
    while (__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY) == RESET)
        if ((uint32_t)(HAL_GetTick() - start) > 100U)
            Error_Handler();
    clocks.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clocks.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clocks.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clocks.APB1CLKDivider = RCC_HCLK_DIV4;
    clocks.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&clocks, FLASH_LATENCY_5) != HAL_OK || HAL_RCC_GetHCLKFreq() != 168000000U)
        Error_Handler();
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2)
        ++app_ticks;
}
void Error_Handler(void) {
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    __disable_irq();
    while (1) {
    }
}
