#include "application.h"
#include "main.h"
#include "render.h"
#include "ssd1306.h"
#include "telemetry.h"
#include <string.h>

static acquisition_t acquisition;
static bool result_available;
static enum { START, CONVERSION, INTERVAL } acquisition_phase;
static uint32_t last_attempt;
static bool button_candidate, candidate_valid, button_stable, button_previous;
static uint32_t candidate_since;
static gui_screen_t screen;
static gui_frame_t last_frame;
static uint64_t uptime;
static uint32_t previous_ticks, display_retry_since;
static bool rendered, pending, display_waiting, display_error;
static bool blinking, blink_level;
static int8_t led_level;
static uint32_t led_transition;

static void publish(sht31d_status_t status) {
    acquisition.status = status;
    if (status == SHT31D_STATUS_TIMEOUT || status == SHT31D_STATUS_COMMUNICATION_ERROR)
        acquisition.i2c_error = true;
    else if (status == SHT31D_STATUS_OK || status == SHT31D_STATUS_CRC_ERROR)
        acquisition.i2c_error = false;
    result_available = true;
    if (status == SHT31D_STATUS_OK)
        (void)telemetry_sample(&acquisition.sample);
    else
        (void)telemetry_error(false, (int32_t)status);
}

static void process_button(void) {
    int level = gpio_pin_get_dt(&button);
    if (level < 0) {
        candidate_valid = false;
        return;
    }
    bool pressed = level != 0; /* GPIO_ACTIVE_LOW is applied by Zephyr. */
    uint32_t now = app_ticks;
    if (!candidate_valid || pressed != button_candidate) {
        button_candidate = pressed;
        candidate_since = now;
        candidate_valid = true;
    } else if ((uint32_t)(now - candidate_since) >= 30U) {
        button_stable = pressed;
    }
    if (button_stable && !button_previous)
        screen = screen == SCREEN_READINGS ? SCREEN_DIAGNOSTICS : SCREEN_READINGS;
    button_previous = button_stable;
}

static void process_acquisition(void) {
    if (acquisition_phase == INTERVAL) {
        if ((uint32_t)(app_ticks - last_attempt) < 2000U)
            return;
        acquisition_phase = START;
    }
    if (acquisition_phase == START) {
        sht31d_status_t status = sht31d_start();
        last_attempt = app_ticks; /* Conversion and retry delays start after the call. */
        acquisition_phase = status == SHT31D_STATUS_OK ? CONVERSION : INTERVAL;
        if (status != SHT31D_STATUS_OK)
            publish(status);
        return;
    }
    if ((uint32_t)(app_ticks - last_attempt) < 16U)
        return;
    sht31d_sample_t sample;
    sht31d_status_t status = sht31d_read(&sample);
    acquisition_phase = INTERVAL;
    if (status == SHT31D_STATUS_OK) {
        acquisition.sample = sample;
        acquisition.acquired_at = app_ticks;
        acquisition.has_sample = true;
    }
    publish(status);
}

static bool frame_changed(const gui_frame_t *frame) {
    if (!rendered || last_frame.screen != frame->screen || last_frame.has_result != frame->has_result)
        return true;
    if (frame->has_result && (last_frame.acquisition.status != frame->acquisition.status ||
                              last_frame.acquisition.has_sample != frame->acquisition.has_sample ||
                              (frame->acquisition.has_sample &&
                               (last_frame.acquisition.sample.temperature != frame->acquisition.sample.temperature ||
                                last_frame.acquisition.sample.humidity != frame->acquisition.sample.humidity))))
        return true;
    return frame->screen == SCREEN_DIAGNOSTICS && last_frame.uptime_seconds != frame->uptime_seconds;
}

static void process_display(void) {
    uint32_t now = app_ticks;
    uptime += (uint32_t)(now - previous_ticks);
    previous_ticks = now;
    if (display_waiting && (uint32_t)(now - display_retry_since) < 1000U)
        return;
    ssd1306_status_t status = SSD1306_STATUS_OK;
    if (!pending) {
        gui_frame_t frame = {.screen = screen,
                             .uptime_seconds = uptime / 1000U,
                             .acquisition = acquisition,
                             .has_result = result_available};
        if (!frame_changed(&frame))
            return;
        status = render_frame(&oled, &frame);
        if (status == SSD1306_STATUS_OK) {
            last_frame = frame;
            rendered = true;
            pending = true;
        } else
            rendered = false;
    }
    if (status == SSD1306_STATUS_OK)
        status = ssd1306_update(&pending);
    display_waiting = status != SSD1306_STATUS_OK;
    if (display_waiting) {
        display_retry_since = app_ticks;
        (void)telemetry_error(true, (int32_t)status);
        if (status == SSD1306_STATUS_TIMEOUT || status == SSD1306_STATUS_COMMUNICATION_ERROR)
            display_error = true;
    } else
        display_error = false;
}

static void process_led(void) {
    bool desired = true; /* application_process only runs after successful startup. */
    if (acquisition.i2c_error || display_error) {
        uint32_t now = app_ticks;
        if (!blinking) {
            blinking = true;
            blink_level = false;
            led_transition = now;
        } else if ((uint32_t)(now - led_transition) >= 500U) {
            blink_level = !blink_level;
            led_transition = now;
        }
        desired = blink_level;
    } else
        blinking = false;
    if (desired != led_level) {
        if (gpio_pin_set_dt(&status_led, desired) == 0)
            led_level = desired;
    }
}

void application_init(void) {
    acquisition = (acquisition_t){0};
    result_available = false;
    acquisition_phase = START;
    last_attempt = 0;
    candidate_valid = button_stable = button_previous = button_candidate = false;
    candidate_since = 0;
    screen = SCREEN_READINGS;
    memset(&last_frame, 0, sizeof(last_frame));
    previous_ticks = app_ticks;
    uptime = previous_ticks;
    rendered = pending = display_waiting = display_error = false;
    display_retry_since = led_transition = 0;
    blinking = blink_level = false;
    led_level = -1;
    telemetry_init();
}

void application_process(void) {
    process_button();
    process_acquisition();
    process_display();
    process_led();
    telemetry_process();
}
