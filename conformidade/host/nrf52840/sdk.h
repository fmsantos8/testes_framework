#ifndef TEST_NRF_SDK_H
#define TEST_NRF_SDK_H
/* Deliberately single-threaded SDK test double; no IRQ scheduling emulation. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#define BIT(n) (UINT32_C(1) << (n))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define BUILD_ASSERT(c,msg) _Static_assert(c,msg)
#define N_LABEL_gpio0 0
#define N_LABEL_gpio1 1
#define N_LABEL_i2c0 2
#define N_LABEL_uart0 3
#define N_LABEL_timer2 4
#define N_ALIAS_hal_i2c0 2
#define N_ALIAS_hal_uart0 3
#define DT_NODELABEL(n) N_LABEL_##n
#define DT_ALIAS(n) N_ALIAS_##n
#define DT_NODE_HAS_STATUS(node,status) 0
#define DT_SAME_NODE(a,b) ((a)==(b))
#define DT_IRQN(node) (node)
#define DT_IRQ(node,field) 5
#define IRQ_CONNECT(irq,priority,handler,arg,flags) ((void)0)
struct gpio_driver_config { uint32_t port_pin_mask; };
struct device { const void *config; int port; };
extern const struct device model_devices[2];
#define DEVICE_DT_GET(node) (&model_devices[node])
bool device_is_ready(const struct device *);
typedef uint32_t gpio_pin_t;
typedef uint32_t gpio_flags_t;
enum { GPIO_INPUT=1, GPIO_OUTPUT_HIGH=2, GPIO_OUTPUT_LOW=4, GPIO_PULL_UP=8, GPIO_PULL_DOWN=16 };
int gpio_pin_configure(const struct device *,gpio_pin_t,gpio_flags_t);
int gpio_pin_get_raw(const struct device *,gpio_pin_t);
int gpio_pin_set_raw(const struct device *,gpio_pin_t,int);
struct k_sem { unsigned count; };
int k_sem_init(struct k_sem *,unsigned,unsigned);
void k_sem_reset(struct k_sem *);
void k_sem_give(struct k_sem *);
int k_sem_take(struct k_sem *,int64_t);
#define K_MSEC(ms) ((int64_t)(ms))
bool k_is_in_isr(void);
int64_t k_uptime_get(void);
void k_msleep(int32_t);
unsigned irq_lock(void);
void irq_unlock(unsigned);
void NVIC_ClearPendingIRQ(int);
extern uint32_t SystemCoreClock;
struct pinctrl_dev_config { unsigned id; };
struct pinctrl_state { uint8_t pin_cnt; const uint32_t *pins; };
extern const struct pinctrl_dev_config model_pinctrl[5];
#define PINCTRL_DT_DEFINE(node)
#define PINCTRL_DT_DEV_CONFIG_GET(node) (&model_pinctrl[node])
#define PINCTRL_STATE_DEFAULT 0
#define NRF_FUN_POS 16
#define NRF_FUN_MSK 255
#define NRF_PIN_POS 0
#define NRF_PIN_MSK 255
enum { NRF_FUN_TWIM_SCL=1, NRF_FUN_TWIM_SDA, NRF_FUN_UART_TX, NRF_FUN_UART_RX };
int pinctrl_lookup_state(const struct pinctrl_dev_config *,int,const struct pinctrl_state **);
int pinctrl_apply_state(const struct pinctrl_dev_config *,int);
typedef int nrfx_err_t;
enum { NRFX_SUCCESS, NRFX_ERROR_NO_MEM, NRFX_ERROR_BUSY, NRFX_ERROR_ALREADY,
       NRFX_ERROR_NOT_SUPPORTED, NRFX_ERROR_TIMEOUT, NRFX_ERROR_INTERNAL };
typedef struct { unsigned instance; } nrfx_twim_t;
#define NRFX_TWIM_INSTANCE(n) {n}
typedef struct { uint32_t scl,sda,frequency; bool skip_gpio_cfg; } nrfx_twim_config_t;
#define NRFX_TWIM_DEFAULT_CONFIG(scl_pin,sda_pin) ((nrfx_twim_config_t){.scl=(scl_pin),.sda=(sda_pin)})
enum { NRF_TWIM_FREQ_100K=100000, NRF_TWIM_FREQ_250K=250000, NRF_TWIM_FREQ_400K=400000,
       NRFX_TWIM_EVT_DONE=0, NRFX_TWIM_EVT_ADDRESS_NACK=1 };
typedef struct { int type; } nrfx_twim_evt_t;
typedef struct { uint8_t address; uint8_t *data; uint32_t size; bool read; } nrfx_twim_xfer_desc_t;
#define NRFX_TWIM_XFER_DESC_RX(a,p,n) {a,p,n,true}
#define NRFX_TWIM_XFER_DESC_TX(a,p,n) {a,(uint8_t *)(p),n,false}
bool nrfx_twim_init_check(const nrfx_twim_t *);
nrfx_err_t nrfx_twim_init(const nrfx_twim_t *,const nrfx_twim_config_t *,void (*)(const nrfx_twim_evt_t *,void *),void *);
void nrfx_twim_enable(const nrfx_twim_t *);
void nrfx_twim_disable(const nrfx_twim_t *);
nrfx_err_t nrfx_twim_xfer(const nrfx_twim_t *,const nrfx_twim_xfer_desc_t *,uint32_t);
struct sys_notify { int value; };
struct onoff_client { struct sys_notify notify; };
struct onoff_manager { int unused; };
#define CLOCK_CONTROL_NRF_SUBSYS_HF 0
struct onoff_manager *z_nrf_clock_control_get_onoff(int);
void sys_notify_init_spinwait(struct sys_notify *);
int sys_notify_fetch_result(struct sys_notify *,int *);
int onoff_request(struct onoff_manager *,struct onoff_client *);
int onoff_cancel_or_release(struct onoff_manager *,struct onoff_client *);
int onoff_release(struct onoff_manager *);
typedef struct { unsigned instance; void *p_reg; } nrfx_uarte_t;
#define NRFX_UARTE_INSTANCE(n) {n,NULL}
typedef struct { uint32_t tx,rx; bool skip_gpio_cfg,skip_psel_cfg; void *p_context; } nrfx_uarte_config_t;
#define NRFX_UARTE_DEFAULT_CONFIG(tx_pin,rx_pin) ((nrfx_uarte_config_t){.tx=(tx_pin),.rx=(rx_pin)})
enum { NRFX_UARTE_EVT_TX_DONE, NRFX_UARTE_TX_DONE_ABORTED=1 };
typedef struct { int type; struct { struct { uint8_t *p_buffer; size_t length; uint32_t flags; } tx; } data; } nrfx_uarte_event_t;
bool nrfx_uarte_init_check(const nrfx_uarte_t *);
nrfx_err_t nrfx_uarte_init(const nrfx_uarte_t *,const nrfx_uarte_config_t *,void (*)(const nrfx_uarte_event_t *,void *));
nrfx_err_t nrfx_uarte_tx(const nrfx_uarte_t *,const uint8_t *,size_t,uint32_t);
nrfx_err_t nrfx_uarte_tx_abort(const nrfx_uarte_t *,bool);
uint32_t nrfy_uarte_tx_amount_get(void *);
typedef int nrf_timer_event_t;
typedef struct { unsigned instance; } nrfx_timer_t;
#define NRFX_TIMER_INSTANCE(n) {n}
typedef struct { uint32_t frequency; int bit_width; void *p_context; } nrfx_timer_config_t;
#define NRFX_TIMER_DEFAULT_CONFIG(freq) ((nrfx_timer_config_t){.frequency=(freq)})
enum { NRF_TIMER_EVENT_COMPARE0, NRF_TIMER_BIT_WIDTH_32=32, NRF_TIMER_CC_CHANNEL0=0, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK=1 };
bool nrfx_timer_init_check(const nrfx_timer_t *);
nrfx_err_t nrfx_timer_init(const nrfx_timer_t *,const nrfx_timer_config_t *,void (*)(nrf_timer_event_t,void *));
void nrfx_timer_extended_compare(const nrfx_timer_t *,int,uint32_t,uint32_t,bool);
void nrfx_timer_clear(const nrfx_timer_t *);
void nrfx_timer_enable(const nrfx_timer_t *);
#endif
