#include "sht31d.h"
#include "main.h"
#include <stdbool.h>
#include <stddef.h>
static bool initialized, measurement_pending;
static sht31d_status_t translate(esp_err_t status) {
    return status == ESP_OK            ? SHT31D_STATUS_OK
           : status == ESP_ERR_TIMEOUT ? SHT31D_STATUS_TIMEOUT
                                       : SHT31D_STATUS_COMMUNICATION_ERROR;
}
static uint8_t word_crc(const uint8_t *bytes) {
    uint8_t crc = 0xFF;
    for (unsigned i = 0; i < 2; ++i) {
        crc ^= bytes[i];
        for (unsigned bit = 0; bit < 8; ++bit)
            crc = (crc & 0x80U) ? (uint8_t)((crc << 1) ^ 0x31U) : (uint8_t)(crc << 1);
    }
    return crc;
}
sht31d_status_t sht31d_init(void) {
    initialized = measurement_pending = false;
    const uint8_t command[] = {0x30, 0x66}; /* Heater off. */
    sht31d_status_t status = translate(i2c_master_transmit(sensor_device, command, sizeof(command), 100));
    initialized = status == SHT31D_STATUS_OK;
    return status;
}
sht31d_status_t sht31d_start(void) {
    if (!initialized)
        return SHT31D_STATUS_NOT_INITIALIZED;
    if (measurement_pending)
        return SHT31D_STATUS_INVALID_STATE;
    const uint8_t command[] = {0x24, 0x00}; /* High repeatability, separate STOP. */
    sht31d_status_t status = translate(i2c_master_transmit(sensor_device, command, sizeof(command), 100));
    measurement_pending = status == SHT31D_STATUS_OK;
    return status;
}
sht31d_status_t sht31d_read(sht31d_sample_t *sample) {
    if (sample == NULL)
        return SHT31D_STATUS_INVALID_ARGUMENT;
    if (!initialized)
        return SHT31D_STATUS_NOT_INITIALIZED;
    if (!measurement_pending)
        return SHT31D_STATUS_INVALID_STATE;
    measurement_pending = false;
    uint8_t bytes[6];
    sht31d_status_t status = translate(i2c_master_receive(sensor_device, bytes, sizeof(bytes), 100));
    if (status != SHT31D_STATUS_OK)
        return status;
    if (word_crc(bytes) != bytes[2] || word_crc(bytes + 3) != bytes[5])
        return SHT31D_STATUS_CRC_ERROR;
    uint32_t raw_t = ((uint32_t)bytes[0] << 8) | bytes[1];
    uint32_t raw_h = ((uint32_t)bytes[3] << 8) | bytes[4];
    *sample = (sht31d_sample_t){.temperature = -45000 + (int32_t)((UINT64_C(175000) * raw_t + 32767U) / 65535U),
                                .humidity = (uint32_t)((UINT64_C(100000) * raw_h + 32767U) / 65535U)};
    return SHT31D_STATUS_OK;
}
