#include "main.h"
#include "application.h"
#include <nrfx_timer.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/drivers/clock_control/nrf_clock_control.h>
#include <string.h>
#include <errno.h>

#define BUS_NODE DT_ALIAS(app_i2c)
#define UART_NODE DT_ALIAS(app_uart)
#define TIMER_NODE DT_NODELABEL(timer2)
BUILD_ASSERT(!DT_NODE_HAS_STATUS(BUS_NODE, okay), "nrfx owns TWIM0");
BUILD_ASSERT(!DT_NODE_HAS_STATUS(UART_NODE, okay), "nrfx owns UARTE0");
BUILD_ASSERT(!DT_NODE_HAS_STATUS(TIMER_NODE, okay), "nrfx owns TIMER2");
PINCTRL_DT_DEFINE(BUS_NODE);
PINCTRL_DT_DEFINE(UART_NODE);
const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(app_button), gpios);
const struct gpio_dt_spec status_led = GPIO_DT_SPEC_GET(DT_ALIAS(app_led), gpios);
static const nrfx_twim_t twim = NRFX_TWIM_INSTANCE(0);
static const nrfx_uarte_t uarte = NRFX_UARTE_INSTANCE(0);
static const nrfx_timer_t timer = NRFX_TIMER_INSTANCE(2);
K_SEM_DEFINE(i2c_done, 0, 1);
K_SEM_DEFINE(uart_done, 0, 1);
static nrfx_err_t i2c_result, uart_result;
static _Alignas(4) uint8_t i2c_dma[256];
static uint8_t uart_dma[64];
static uint32_t uart_pending, uart_transferred;
static bool uart_active, uart_available;
static struct onoff_client clock_request;

static void i2c_event(const nrfx_twim_evt_t *event, void *context) {
    (void)context;
    i2c_result = event->type == NRFX_TWIM_EVT_DONE ? NRFX_SUCCESS : NRFX_ERROR_INTERNAL;
    k_sem_give(&i2c_done);
}
static void uart_event(const nrfx_uarte_event_t *event, void *context) {
    (void)context;
    if (event->type != NRFX_UARTE_EVT_TX_DONE || event->data.tx.p_buffer != uart_dma)
        return;
    unsigned key = irq_lock();
    if (uart_active) {
        uart_transferred = (uint32_t)MIN(event->data.tx.length, uart_pending);
        uart_result = event->data.tx.flags & NRFX_UARTE_TX_DONE_ABORTED ? NRFX_ERROR_INTERNAL : NRFX_SUCCESS;
        uart_active = false;
        k_sem_give(&uart_done);
    }
    irq_unlock(key);
}
static void timer_event(nrf_timer_event_t event, void *context) {
    (void)context;
    if (event == NRF_TIMER_EVENT_COMPARE0)
        ++app_ticks;
}
static bool request_hfxo(void) {
    struct onoff_manager *clock = z_nrf_clock_control_get_onoff(CLOCK_CONTROL_NRF_SUBSYS_HF);
    if (!clock)
        return false;
    sys_notify_init_spinwait(&clock_request.notify);
    if (onoff_request(clock, &clock_request) < 0)
        return false;
    int result;
    int64_t deadline = k_uptime_get() + 100;
    while (sys_notify_fetch_result(&clock_request.notify, &result) != 0) {
        if (k_uptime_get() >= deadline) {
            (void)onoff_cancel_or_release(clock, &clock_request);
            return false;
        }
        k_msleep(1);
    }
    if (result < 0) {
        (void)onoff_cancel_or_release(clock, &clock_request);
        return false;
    }
    return true; /* Retain the clock reference for the entire firmware lifetime. */
}
int peripherals_init(void) {
    if (!gpio_is_ready_dt(&button) || !gpio_is_ready_dt(&status_led))
        return -ENODEV;
    if (gpio_pin_configure_dt(&status_led, GPIO_OUTPUT_INACTIVE) || gpio_pin_configure_dt(&button, GPIO_INPUT))
        return -EIO;
    if (pinctrl_apply_state(PINCTRL_DT_DEV_CONFIG_GET(BUS_NODE), PINCTRL_STATE_DEFAULT) ||
        pinctrl_apply_state(PINCTRL_DT_DEV_CONFIG_GET(UART_NODE), PINCTRL_STATE_DEFAULT))
        return -EIO;
    nrfx_twim_config_t bus = NRFX_TWIM_DEFAULT_CONFIG(11, 8);
    bus.frequency = NRF_TWIM_FREQ_100K;
    bus.skip_gpio_cfg = true;
    IRQ_CONNECT(DT_IRQN(BUS_NODE), DT_IRQ(BUS_NODE, priority), nrfx_twim_0_irq_handler, 0, 0);
    if (nrfx_twim_init(&twim, &bus, i2c_event, NULL) != NRFX_SUCCESS)
        return -EIO;
    nrfx_twim_enable(&twim);
    if (!request_hfxo())
        return -EIO;
    nrfx_uarte_config_t serial = NRFX_UARTE_DEFAULT_CONFIG(20, 22);
    serial.skip_gpio_cfg = true;
    serial.skip_psel_cfg = true;
    IRQ_CONNECT(DT_IRQN(UART_NODE), DT_IRQ(UART_NODE, priority), nrfx_uarte_0_irq_handler, 0, 0);
    if (nrfx_uarte_init(&uarte, &serial, uart_event) != NRFX_SUCCESS)
        return -EIO;
    uart_available = true;
    nrfx_timer_config_t tick = NRFX_TIMER_DEFAULT_CONFIG(1000000);
    tick.bit_width = NRF_TIMER_BIT_WIDTH_32;
    IRQ_CONNECT(DT_IRQN(TIMER_NODE), DT_IRQ(TIMER_NODE, priority), nrfx_timer_2_irq_handler, 0, 0);
    if (nrfx_timer_init(&timer, &tick, timer_event) != NRFX_SUCCESS)
        return -EIO;
    nrfx_timer_extended_compare(&timer, NRF_TIMER_CC_CHANNEL0, 1000, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
    return 0;
}
void application_timer_start(void) {
    unsigned key = irq_lock();
    nrfx_timer_clear(&timer);
    NVIC_ClearPendingIRQ(DT_IRQN(TIMER_NODE));
    nrfx_timer_enable(&timer);
    irq_unlock(key);
}
nrfx_err_t twim_transfer(uint8_t address, uint8_t *bytes, uint32_t size, bool read) {
    if (!bytes || !size || size > sizeof(i2c_dma))
        return NRFX_ERROR_INVALID_PARAM;
    if (!read)
        memcpy(i2c_dma, bytes, size);
    k_sem_reset(&i2c_done);
    nrfx_twim_xfer_desc_t xfer = read ? (nrfx_twim_xfer_desc_t)NRFX_TWIM_XFER_DESC_RX(address, i2c_dma, size)
                                      : (nrfx_twim_xfer_desc_t)NRFX_TWIM_XFER_DESC_TX(address, i2c_dma, size);
    nrfx_err_t result = nrfx_twim_xfer(&twim, &xfer, 0);
    if (result != NRFX_SUCCESS)
        return result;
    if (k_sem_take(&i2c_done, K_MSEC(100)) != 0) {
        unsigned key = irq_lock();
        nrfx_twim_disable(&twim);
        NVIC_ClearPendingIRQ(DT_IRQN(BUS_NODE));
        nrfx_twim_enable(&twim);
        irq_unlock(key);
        return NRFX_ERROR_TIMEOUT;
    }
    if (i2c_result == NRFX_SUCCESS && read)
        memcpy(bytes, i2c_dma, size);
    return i2c_result;
}
nrfx_err_t uarte_transmit(const uint8_t *bytes, uint32_t size, uint32_t *transmitted) {
    *transmitted = 0;
    if (!uart_available)
        return NRFX_ERROR_INVALID_STATE;
    int64_t deadline = k_uptime_get() + 1000;
    while (*transmitted < size) {
        int64_t remaining = deadline - k_uptime_get();
        if (remaining <= 0)
            return NRFX_ERROR_TIMEOUT;
        uint32_t chunk = MIN(size - *transmitted, sizeof(uart_dma));
        k_sem_reset(&uart_done);
        unsigned key = irq_lock();
        memcpy(uart_dma, bytes + *transmitted, chunk);
        uart_pending = chunk;
        uart_transferred = 0;
        uart_result = NRFX_ERROR_INTERNAL;
        uart_active = true;
        irq_unlock(key);
        nrfx_err_t result = nrfx_uarte_tx(&uarte, uart_dma, chunk, 0);
        if (result != NRFX_SUCCESS) {
            key = irq_lock();
            uart_active = false;
            irq_unlock(key);
            return result;
        }
        if (k_sem_take(&uart_done, K_MSEC(remaining)) != 0) {
            (void)nrfx_uarte_tx_abort(&uarte, false);
            (void)k_sem_take(&uart_done, K_MSEC(10));
            key = irq_lock();
            if (uart_active) {
                uart_transferred = (uint32_t)MIN(nrfy_uarte_tx_amount_get(uarte.p_reg), uart_pending);
                uart_active = false;
                uart_available = false; /* Never reuse DMA storage while STOPTX is unconfirmed. */
            }
            *transmitted += uart_transferred;
            irq_unlock(key);
            return NRFX_ERROR_TIMEOUT;
        }
        *transmitted += uart_transferred;
        if (uart_result != NRFX_SUCCESS || uart_transferred != chunk)
            return NRFX_ERROR_INTERNAL;
    }
    return NRFX_SUCCESS;
}
