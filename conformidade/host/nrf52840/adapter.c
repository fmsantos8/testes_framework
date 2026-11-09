#include "fixture.h"
#include "sdk.h"
#include <assert.h>
uint32_t SystemCoreClock=64000000;
static bool levels[2][32], bus_ready, uart_ready, timer_ready;
static int64_t uptime;
static const struct gpio_driver_config gpio_configs[2]={{UINT32_MAX},{UINT16_MAX}};
const struct device model_devices[2]={{&gpio_configs[0],0},{&gpio_configs[1],1}};
const struct pinctrl_dev_config model_pinctrl[5]={{0},{1},{2},{3},{4}};
static const uint32_t bus_pins[]={11|(NRF_FUN_TWIM_SCL<<16),8|(NRF_FUN_TWIM_SDA<<16)};
static const uint32_t uart_pins[]={20|(NRF_FUN_UART_TX<<16),22|(NRF_FUN_UART_RX<<16)};
static const struct pinctrl_state bus_state={2,bus_pins}, uart_state={2,uart_pins};
static void (*bus_callback)(const nrfx_twim_evt_t *,void *);
static void *bus_context;
static void (*uart_callback)(const nrfx_uarte_event_t *,void *);
static void *uart_context;
static uint8_t *uart_buffer;
static void (*timer_callback)(nrf_timer_event_t,void *);
static void *timer_context;
void fixture_setup(void) {
    fixture_cpu=64000000;fixture_i2c_limit=256;
    fixture_input=(hal_gpio_config_t){{0,6},HAL_GPIO_INPUT,HAL_GPIO_PULL_NONE,false};
    fixture_output=(hal_gpio_config_t){{0,17},HAL_GPIO_OUTPUT,HAL_GPIO_PULL_NONE,false};
    fixture_bus=(hal_i2c_config_t){0,{0,11},{0,8},100000,100};
    fixture_serial=(hal_uart_config_t){0,{0,20},{0,22},115200,HAL_UART_DATA_WIDTH_8,HAL_UART_PARITY_NONE,HAL_UART_STOP_COUNT_1,1000};
    fixture_timer=(hal_timer_config_t){.instance=2,.period=1};
}
void fixture_set_input(bool level){levels[0][6]=level;}
bool fixture_output_level(void){return levels[0][17];}
void fixture_fire_timer(void){assert(timer_callback);timer_callback(NRF_TIMER_EVENT_COMPARE0,timer_context);}
bool fixture_unsupported_timer(hal_timer_config_t *config){config->instance=0;return true;}
bool device_is_ready(const struct device *device){(void)device;return true;}
int gpio_pin_configure(const struct device *device,gpio_pin_t pin,gpio_flags_t flags){if(flags&GPIO_OUTPUT_HIGH)levels[device->port][pin]=true;if(flags&GPIO_OUTPUT_LOW)levels[device->port][pin]=false;return 0;}
int gpio_pin_get_raw(const struct device *device,gpio_pin_t pin){return levels[device->port][pin];}
int gpio_pin_set_raw(const struct device *device,gpio_pin_t pin,int level){levels[device->port][pin]=level!=0;return 0;}
int k_sem_init(struct k_sem *sem,unsigned initial,unsigned limit){(void)limit;sem->count=initial;return 0;}
void k_sem_reset(struct k_sem *sem){sem->count=0;}
void k_sem_give(struct k_sem *sem){sem->count=1;}
int k_sem_take(struct k_sem *sem,int64_t timeout){
    if((void *)sem==bus_context)fixture_last_timeout=(uint32_t)timeout;
    if(sem->count){--sem->count;return 0;}uptime+=timeout;return -1;
}
bool k_is_in_isr(void){return false;}
int64_t k_uptime_get(void){return uptime;}
void k_msleep(int32_t ms){uptime+=ms;}
unsigned irq_lock(void){return 0;}
void irq_unlock(unsigned key){(void)key;}
void NVIC_ClearPendingIRQ(int irq){(void)irq;}
int pinctrl_lookup_state(const struct pinctrl_dev_config *config,int state,const struct pinctrl_state **out){(void)state;*out=config->id==2?&bus_state:&uart_state;return 0;}
int pinctrl_apply_state(const struct pinctrl_dev_config *config,int state){(void)config;(void)state;return 0;}
bool nrfx_twim_init_check(const nrfx_twim_t *device){(void)device;return bus_ready;}
nrfx_err_t nrfx_twim_init(const nrfx_twim_t *device,const nrfx_twim_config_t *config,void (*callback)(const nrfx_twim_evt_t *,void *),void *context){(void)device;assert(config->frequency==100000);if(fixture_bus_init_failure)return NRFX_ERROR_INTERNAL;bus_callback=callback;bus_context=context;bus_ready=true;return NRFX_SUCCESS;}
void nrfx_twim_enable(const nrfx_twim_t *device){(void)device;}
void nrfx_twim_disable(const nrfx_twim_t *device){(void)device;}
nrfx_err_t nrfx_twim_xfer(const nrfx_twim_t *device,const nrfx_twim_xfer_desc_t *request,uint32_t flags){
    (void)device;assert(flags==0);fixture_last_address=request->address;
    if(fixture_bus_mode==F_TIMEOUT)return NRFX_SUCCESS;
    if(fixture_bus_mode==F_OK&&request->read)memset(request->data,0x5a,request->size);
    nrfx_twim_evt_t event={fixture_bus_mode==F_OK?NRFX_TWIM_EVT_DONE:NRFX_TWIM_EVT_ADDRESS_NACK};
    bus_callback(&event,bus_context);return NRFX_SUCCESS;
}
struct onoff_manager *z_nrf_clock_control_get_onoff(int clock){(void)clock;static struct onoff_manager manager;return &manager;}
void sys_notify_init_spinwait(struct sys_notify *notify){notify->value=0;}
int sys_notify_fetch_result(struct sys_notify *notify,int *value){*value=notify->value;return 0;}
int onoff_request(struct onoff_manager *manager,struct onoff_client *client){(void)manager;client->notify.value=0;return 0;}
int onoff_cancel_or_release(struct onoff_manager *manager,struct onoff_client *client){(void)manager;(void)client;return 0;}
int onoff_release(struct onoff_manager *manager){(void)manager;return 0;}
bool nrfx_uarte_init_check(const nrfx_uarte_t *device){(void)device;return uart_ready;}
nrfx_err_t nrfx_uarte_init(const nrfx_uarte_t *device,const nrfx_uarte_config_t *config,void (*callback)(const nrfx_uarte_event_t *,void *)){
    (void)device;uart_context=config->p_context;uart_callback=callback;uart_ready=true;return NRFX_SUCCESS;
}
nrfx_err_t nrfx_uarte_tx(const nrfx_uarte_t *device,const uint8_t *data,size_t size,uint32_t flags){
    (void)device;(void)flags;uart_buffer=(uint8_t *)data;
    if(fixture_uart_mode==F_ERROR)return NRFX_ERROR_INTERNAL;
    if(fixture_uart_mode!=F_OK)return NRFX_SUCCESS;
    nrfx_uarte_event_t event={.type=NRFX_UARTE_EVT_TX_DONE,.data.tx={uart_buffer,size,0}};
    uart_callback(&event,uart_context);return NRFX_SUCCESS;
}
nrfx_err_t nrfx_uarte_tx_abort(const nrfx_uarte_t *device,bool synchronous){
    (void)device;assert(!synchronous);
    nrfx_uarte_event_t event={.type=NRFX_UARTE_EVT_TX_DONE,.data.tx={uart_buffer,fixture_uart_mode==F_PARTIAL?2U:0U,NRFX_UARTE_TX_DONE_ABORTED}};
    uart_callback(&event,uart_context);return NRFX_SUCCESS;
}
uint32_t nrfy_uarte_tx_amount_get(void *reg){(void)reg;return 0;}
bool nrfx_timer_init_check(const nrfx_timer_t *device){(void)device;return timer_ready;}
nrfx_err_t nrfx_timer_init(const nrfx_timer_t *device,const nrfx_timer_config_t *config,void (*callback)(nrf_timer_event_t,void *)){
    (void)device;assert(config->frequency==1000000&&config->bit_width==32);timer_context=config->p_context;timer_callback=callback;timer_ready=true;return NRFX_SUCCESS;
}
void nrfx_timer_extended_compare(const nrfx_timer_t *device,int channel,uint32_t count,uint32_t shortcut,bool interrupt){(void)device;(void)channel;(void)shortcut;assert(count==1000&&interrupt);}
void nrfx_timer_clear(const nrfx_timer_t *device){(void)device;}
void nrfx_timer_enable(const nrfx_timer_t *device){(void)device;}
