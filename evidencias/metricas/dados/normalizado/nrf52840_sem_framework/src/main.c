#include "main.h"
#include "application.h"
#include "sht31d.h"
#include "ssd1306.h"
volatile uint32_t app_ticks;
int main(void) {
    if (peripherals_init() != 0)
        k_panic();
    k_msleep(50); /* External VCC rail must settle before device initialization. */
    if (sht31d_init() != SHT31D_STATUS_OK || ssd1306_init() != SSD1306_STATUS_OK)
        k_panic();
    application_init();
    application_timer_start();
    while (true) {
        application_process();
        k_msleep(1);
    }
    return 0;
}
