#include "system/hal_system.h"
#include "hal_esp32_private.h"

#include "hal_memory.h"

#include "esp_clk_tree.h"
#include "esp_pm.h"
#include "sdkconfig.h"

hal_status_t hal_system_init(const hal_system_config_t *config) {
    hal_memory_info_t info;
    if (hal_memory_get_info(&info) == HAL_MEMORY_OK) {
        return HAL_STATUS_INVALID_STATE;
    }
    if (config == NULL || config->memory_buffer == NULL || config->memory_capacity == 0 || config->cpu_clock == 0 ||
        (uintptr_t)config->memory_buffer % _Alignof(max_align_t) != 0) {
        return HAL_STATUS_INVALID_ARGUMENT;
    }

    if (config->cpu_clock != 80000000U && config->cpu_clock != 160000000U && config->cpu_clock != 240000000U) {
        return HAL_STATUS_NOT_SUPPORTED;
    }
#ifdef CONFIG_PM_ENABLE
    const esp_pm_config_t clock_config = {
        .max_freq_mhz = (int)(config->cpu_clock / 1000000U),
        .min_freq_mhz = (int)(config->cpu_clock / 1000000U),
        .light_sleep_enable = false,
    };
    esp_err_t error = esp_pm_configure(&clock_config);
    if (error == ESP_ERR_NOT_SUPPORTED || error == ESP_ERR_INVALID_ARG) {
        return HAL_STATUS_NOT_SUPPORTED;
    }
    if (error != ESP_OK) {
        return HAL_STATUS_ERROR;
    }
#else
    // ESP-IDF startup already selected the fixed CPU frequency in this build.
    if (config->cpu_clock != (uint32_t)CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ * 1000000U) {
        return HAL_STATUS_NOT_SUPPORTED;
    }
#endif
    uint32_t actual_clock = 0;
    if (esp_clk_tree_src_get_freq_hz(SOC_MOD_CLK_CPU, ESP_CLK_TREE_SRC_FREQ_PRECISION_EXACT, &actual_clock) != ESP_OK ||
        actual_clock != config->cpu_clock) {
        return HAL_STATUS_ERROR;
    }

    return hal_esp32_memory_status(hal_memory_init(config->memory_buffer, config->memory_capacity));
}

hal_status_t hal_system_seal(void) {
    return hal_esp32_memory_status(hal_memory_seal());
}
