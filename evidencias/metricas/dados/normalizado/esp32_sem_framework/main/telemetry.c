#include "telemetry.h"
#include "application.h"
#include "main.h"
#include "driver/uart.h"
#include "freertos/task.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

typedef enum { TX_OK, TX_TIMEOUT, TX_ERROR, TX_UNAVAILABLE } tx_result_t;
static tx_result_t transmit_record(const uint8_t *data, uint32_t size, uint32_t *transmitted) {
    *transmitted = 0;
    TickType_t start = xTaskGetTickCount(), budget = pdMS_TO_TICKS(1000);
    while (*transmitted < size) {
        if ((TickType_t)(xTaskGetTickCount() - start) >= budget)
            return TX_TIMEOUT;
        int accepted = uart_tx_chars(UART_NUM_0, (const char *)data + *transmitted, size - *transmitted);
        if (accepted < 0 || (uint32_t)accepted > size - *transmitted)
            return TX_ERROR;
        *transmitted += (uint32_t)accepted;
        if (accepted == 0)
            vTaskDelay(1);
    }
    TickType_t elapsed = xTaskGetTickCount() - start;
    esp_err_t status = uart_wait_tx_done(UART_NUM_0, elapsed < budget ? budget - elapsed : 0);
    return status == ESP_OK ? TX_OK : status == ESP_ERR_TIMEOUT ? TX_TIMEOUT : TX_ERROR;
}
typedef struct {
    bool error, display;
    int32_t code;
    sht31d_sample_t sample;
} record_t;
static record_t queue[8];
static unsigned head, count;
static bool retry_waiting, resync_pending;
static uint32_t retry_since;

void telemetry_init(void) {
    head = count = 0;
    retry_waiting = resync_pending = false;
    retry_since = 0;
}
bool telemetry_sample(const sht31d_sample_t *sample) {
    if (!sample || count == 8)
        return false;
    queue[(head + count) % 8] = (record_t){.sample = *sample};
    ++count;
    return true;
}
bool telemetry_error(bool display, int32_t code) {
    if (count == 8)
        return false;
    queue[(head + count) % 8] = (record_t){.error = true, .display = display, .code = code};
    ++count;
    return true;
}
void telemetry_process(void) {
    if (count == 0 && !resync_pending)
        return;
    if (retry_waiting && (uint32_t)(app_ticks - retry_since) < 1000U)
        return;
    uint32_t transmitted = 0;
    if (resync_pending) {
        tx_result_t result = transmit_record((const uint8_t *)"\r\n", 2, &transmitted);
        if (result == TX_OK) {
            resync_pending = false;
            retry_waiting = false;
        } else {
            retry_since = app_ticks;
            retry_waiting = true;
        }
        return;
    }
    const record_t *record = &queue[head];
    char bytes[128];
    int length = record->error ? snprintf(bytes, sizeof(bytes), "ERROR;source=%s;code=%" PRId32 "\r\n",
                                          record->display ? "SSD1306" : "SHT31D", record->code)
                               : snprintf(bytes, sizeof(bytes),
                                          "SAMPLE;temperature_mC=%" PRId32 ";humidity_mpercent=%" PRIu32 "\r\n",
                                          record->sample.temperature, record->sample.humidity);
    tx_result_t result = length < 0 || (unsigned)length >= sizeof(bytes)
                             ? TX_UNAVAILABLE
                             : transmit_record((const uint8_t *)bytes, (uint32_t)length, &transmitted);
    if (result == TX_OK || transmitted != 0 || (result != TX_TIMEOUT && result != TX_ERROR)) {
        head = (head + 1) % 8;
        --count;
    }
    if (result == TX_OK) {
        retry_waiting = false;
        return;
    }
    resync_pending = transmitted != 0;
    retry_since = app_ticks;
    retry_waiting = true;
}
