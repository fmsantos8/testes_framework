/**
 * @file hal_pin.h
 * @author Fabricio Santos (fmsantos8@ucs.br)
 * @brief Portable physical pin identification.
 *
 */
#ifndef HAL_PIN_H
#define HAL_PIN_H

#include <stdint.h>

/**
 * @brief Physical pin identifier supplied by the board integration.
 *
 * Bank identifiers are zero-based: ESP32 uses 0; STM32 A/B use 0/1; nRF52840
 * P0/P1 use 0/1. number is the bank's pin index, not a mask or board/package label.
 * The BSP defines mappings and wiring; the port validates pin capabilities.
 * Identical identifiers may have different wiring across boards.
 */
typedef struct {
    uint8_t port;   /** Zero-based GPIO bank identifier */
    uint8_t number; /** Zero-based pin index within the bank */
} hal_pin_t;

#endif
