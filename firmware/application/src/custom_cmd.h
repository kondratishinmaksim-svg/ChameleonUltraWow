#ifndef CUSTOM_CMD_H
#define CUSTOM_CMD_H

#include <stdint.h>
#include "utils.h"

// ******************************************************************
//             Custom Animation Commands for Chameleon Ultra
// ******************************************************************

#define DATA_CMD_GET_CUSTOM_ANIM_CONFIG         (1100)
#define DATA_CMD_SET_CUSTOM_ANIM_CONFIG         (1101)
#define DATA_CMD_TRIGGER_TEST_ANIM              (1102)

// Animation effect identifiers
typedef enum {
    ANIM_EFFECT_NONE            = 0,
    ANIM_EFFECT_PULSE_WHITE     = 10,  // Пульсация белым при прикладывании
    ANIM_EFFECT_PULSE_COLOR     = 11,  // Пульсация выбранным цветом
    ANIM_EFFECT_SWEEP_WAVE      = 12,  // Бегущая волна по светодиодам
    ANIM_EFFECT_BREATHE         = 13,  // Плавное дыхание (fade-in / fade-out)
    ANIM_EFFECT_RAINBOW         = 14,  // Цикличный радужный перебор цветов
    ANIM_EFFECT_SLOT_CHASE      = 15,  // Бегущий огонёк по слотам
    ANIM_EFFECT_SPARKLE         = 16,  // Случайные мерцающие искры
    ANIM_EFFECT_SOLID           = 17,  // Статичное свечение
    ANIM_EFFECT_SYMMETRIC_BURST = 18,  // Симметричная вспышка из центра к краям
    ANIM_EFFECT_MAX             = 19
} custom_anim_effect_t;

// 10-byte packed configuration structure
typedef struct PACKED {
    uint8_t on_connect_effect;    // Эффект при считывании/прикладывании (custom_anim_effect_t)
    uint8_t on_connect_color;     // Цвет: 0=Red, 1=Green, 2=Blue, 3=White, 4=Magenta, 5=Yellow, 6=Cyan
    uint8_t on_connect_speed;     // Скорость анимации: 1 (медленно) - 10 (быстро)
    uint8_t on_disconnect_effect; // Эффект при завершении/убирании карты
    uint8_t on_disconnect_color;  // Цвет при убирании карты
    uint8_t on_disconnect_speed;  // Скорость при убирании карты
    uint8_t idle_effect;          // Фоновый эффект (в режиме ожидания)
    uint8_t idle_color;           // Цвет фонового эффекта
    uint8_t idle_speed;           // Скорость фонового эффекта
    uint8_t brightness;           // Яркость: 1 - 99
} custom_anim_config_t;

#endif // CUSTOM_CMD_H
