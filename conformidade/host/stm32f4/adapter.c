#define _GNU_SOURCE
#include "fixture.h"
#include "hal_stm32f4_private.h"
#include <assert.h>
#include <string.h>
#include <sys/mman.h>
uint32_t test_interrupt_mask;
static uint32_t core_clock=16000000, pclk1=16000000, pclk2=16000000;
void fixture_setup(void) {
    assert(mmap((void *)PERIPH_BASE,0x40000,PROT_READ|PROT_WRITE,
        MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)==(void *)PERIPH_BASE);
    fixture_i2c_limit=UINT16_MAX;
#if defined(STM32F411xE)
    fixture_cpu=100000000;
    fixture_input=(hal_gpio_config_t){{1,0},HAL_GPIO_INPUT,HAL_GPIO_PULL_NONE,false};
    fixture_output=(hal_gpio_config_t){{1,1},HAL_GPIO_OUTPUT,HAL_GPIO_PULL_NONE,false};
    fixture_bus=(hal_i2c_config_t){0,{1,6},{1,7},100000,100};
#else
    fixture_cpu=168000000;
    fixture_input=(hal_gpio_config_t){{0,0},HAL_GPIO_INPUT,HAL_GPIO_PULL_NONE,false};
    fixture_output=(hal_gpio_config_t){{0,1},HAL_GPIO_OUTPUT,HAL_GPIO_PULL_NONE,false};
    fixture_bus=(hal_i2c_config_t){0,{1,8},{1,9},100000,100};
#endif
    fixture_serial=(hal_uart_config_t){1,{0,2},{0,3},115200,HAL_UART_DATA_WIDTH_8,HAL_UART_PARITY_NONE,HAL_UART_STOP_COUNT_1,1000};
    fixture_timer=(hal_timer_config_t){.instance=0,.period=1};
}
void fixture_set_input(bool level) {hal_stm32f4_pin_port(fixture_input.pin)->IDR=level?1U:0U;}
bool fixture_output_level(void) {return hal_stm32f4_pin_port(fixture_output.pin)->BSRR==2;}
void TIM2_IRQHandler(void);
void fixture_fire_timer(void) {
    assert(((uint64_t)TIM2->PSC+1)*((uint64_t)TIM2->ARR+1)==(uint64_t)pclk1*2/1000);
    TIM2->SR=TIM_SR_UIF;TIM2_IRQHandler();
}
bool fixture_unsupported_timer(hal_timer_config_t *config) {config->instance=1;config->period=UINT32_MAX;return true;}
void SystemCoreClockUpdate(void) {}
HAL_StatusTypeDef HAL_Init(void) {return HAL_OK;}
void HAL_IncTick(void) {}
uint32_t HAL_GetTick(void) {static uint32_t tick;return tick++;}
uint32_t HAL_RCC_GetPCLK1Freq(void) {return pclk1;}
uint32_t HAL_RCC_GetPCLK2Freq(void) {return pclk2;}
uint32_t HAL_RCC_GetHCLKFreq(void) {return core_clock;}
HAL_StatusTypeDef HAL_RCC_OscConfig(const RCC_OscInitTypeDef *config) {
    assert(config->OscillatorType==RCC_OSCILLATORTYPE_HSI);
    if(config->PLL.PLLState==RCC_PLL_ON){
        assert(config->PLL.PLLSource==RCC_PLLSOURCE_HSI&&config->PLL.PLLM==16);
        PWR->CSR|=PWR_CSR_VOSRDY;
    }
    return HAL_OK;
}
HAL_StatusTypeDef HAL_RCC_ClockConfig(const RCC_ClkInitTypeDef *config,uint32_t latency) {
    (void)latency;
    if(config->SYSCLKSource==RCC_SYSCLKSOURCE_PLLCLK){
        core_clock=fixture_cpu;
#if defined(STM32F411xE)
        pclk1=core_clock/2;pclk2=core_clock;RCC->CFGR=RCC_CFGR_PPRE1_DIV2;
#else
        pclk1=core_clock/4;pclk2=core_clock/2;RCC->CFGR=RCC_CFGR_PPRE1_DIV4|RCC_CFGR_PPRE2_DIV2;
#endif
    }
    return HAL_OK;
}
void HAL_NVIC_SetPriority(IRQn_Type irq,uint32_t priority,uint32_t subpriority) {(void)irq;assert(priority==5&&subpriority==0);}
void HAL_NVIC_EnableIRQ(IRQn_Type irq) {(void)irq;}
void HAL_NVIC_ClearPendingIRQ(IRQn_Type irq) {(void)irq;}
HAL_StatusTypeDef __real_HAL_I2C_Init(I2C_HandleTypeDef *);
HAL_StatusTypeDef __wrap_HAL_I2C_Init(I2C_HandleTypeDef *device) {return fixture_bus_init_failure?HAL_ERROR:__real_HAL_I2C_Init(device);}
static HAL_StatusTypeDef bus_result(I2C_HandleTypeDef *device) {
    device->ErrorCode=fixture_bus_mode==F_TIMEOUT?HAL_I2C_ERROR_TIMEOUT:fixture_bus_mode==F_ERROR?HAL_I2C_ERROR_AF:0;
    return fixture_bus_mode==F_OK?HAL_OK:fixture_bus_mode==F_TIMEOUT?HAL_TIMEOUT:HAL_ERROR;
}
HAL_StatusTypeDef __wrap_HAL_I2C_Master_Transmit(I2C_HandleTypeDef *device,uint16_t address,uint8_t *data,uint16_t size,uint32_t timeout) {
    (void)data;(void)size;assert((address&1)==0);fixture_last_address=address>>1;fixture_last_timeout=timeout;return bus_result(device);
}
HAL_StatusTypeDef __wrap_HAL_I2C_Master_Receive(I2C_HandleTypeDef *device,uint16_t address,uint8_t *data,uint16_t size,uint32_t timeout) {
    fixture_last_address=address>>1;fixture_last_timeout=timeout;if(fixture_bus_mode==F_OK)memset(data,0x5a,size);return bus_result(device);
}
HAL_StatusTypeDef __wrap_HAL_UART_Transmit(UART_HandleTypeDef *device,const uint8_t *data,uint16_t size,uint32_t timeout) {
    (void)data;assert(timeout==fixture_serial.timeout);
    uint32_t accepted=fixture_uart_mode==F_OK?size:fixture_uart_mode==F_PARTIAL?2:0;
    device->TxXferCount=size-accepted;
    return fixture_uart_mode==F_OK?HAL_OK:fixture_uart_mode==F_ERROR?HAL_ERROR:HAL_TIMEOUT;
}
