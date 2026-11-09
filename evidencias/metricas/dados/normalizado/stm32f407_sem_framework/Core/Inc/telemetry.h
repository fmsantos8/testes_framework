#ifndef TELEMETRY_H
#define TELEMETRY_H
#include "sht31d.h"
#include <stdbool.h>
#include <stdint.h>
void telemetry_init(void);
bool telemetry_sample(const sht31d_sample_t *sample);
bool telemetry_error(bool display, int32_t code);
void telemetry_process(void);
#endif
