#ifndef APPLICATION_H
#define APPLICATION_H
#include "sht31d.h"
#include <stdbool.h>
#include <stdint.h>
/* Wire status values deliberately match the reference firmware. */
typedef struct {
    sht31d_status_t status;
    sht31d_sample_t sample;
    uint32_t acquired_at;
    bool has_sample;
    bool i2c_error;
} acquisition_t;
extern volatile uint32_t app_ticks; /* Written only by the physical 1 ms timer ISR. */
void application_init(void);
void application_process(void);
#endif
