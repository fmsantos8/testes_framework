#ifndef SHT31D_H
#define SHT31D_H
#include <stdint.h>
typedef enum {
    SHT31D_STATUS_OK = 0,
    SHT31D_STATUS_INVALID_ARGUMENT = 1,
    SHT31D_STATUS_NOT_INITIALIZED = 2,
    SHT31D_STATUS_INVALID_STATE = 3,
    SHT31D_STATUS_NOT_SUPPORTED = 4,
    SHT31D_STATUS_TIMEOUT = 5,
    SHT31D_STATUS_COMMUNICATION_ERROR = 6,
    SHT31D_STATUS_CRC_ERROR = 7
} sht31d_status_t;
typedef struct {
    int32_t temperature;
    uint32_t humidity;
} sht31d_sample_t;
sht31d_status_t sht31d_init(void);
sht31d_status_t sht31d_start(void);
sht31d_status_t sht31d_read(sht31d_sample_t *sample);
#endif
