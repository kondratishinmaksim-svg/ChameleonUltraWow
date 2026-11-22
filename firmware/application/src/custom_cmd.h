#ifndef CUSTOM_CMD_H
#define CUSTOM_CMD_H

#include <stdint.h>
#include "utils.h"

#define DATA_CMD_GET_CUSTOM_ANIM_CONFIG         (1100)
#define DATA_CMD_SET_CUSTOM_ANIM_CONFIG         (1101)
#define DATA_CMD_TRIGGER_TEST_ANIM              (1102)

typedef enum {
    ANIM_EFFECT_NONE            = 0,
    ANIM_EFFECT_PULSE_WHITE     = 10,  // Чистая белая пульсация при касании
    ANIM_EFFECT_PULSE_COLOR     = 11,  // Пульсация выбранным цветом
    ANIM_EFFECT_SWEEP_WAVE      = 12,  // Бегущая волна по светодиодам
    ANIM_EFFECT_BREATHE         = 13,  // Плавное дыхание (fade-in / fade-out)
    ANIM_EFFECT_RAINBOW         = 14,  // Цикличный радужный перебор цветов
    ANIM_EFFECT_SLOT_CHASE      = 15,  // Бегущий огонёк а-ля Knight Rider
    ANIM_EFFECT_SPARKLE         = 16,  // Случайные мерцающие искры
    ANIM_EFFECT_SOLID           = 17,  // Статичное свечение
    ANIM_EFFECT_SYMMETRIC_BURST = 18,  // Симметричная вспышка из центра
    ANIM_EFFECT_STROBE          = 19,  // Быстрый стробоскоп
    ANIM_EFFECT_HEARTBEAT       = 20,  // Двойной пульс (сердцебиение)
    ANIM_EFFECT_POLICE          = 21,  // Полицейская мигалка (красно-синий)
    ANIM_EFFECT_FIRE            = 22,  // Живое пламя костра
    ANIM_EFFECT_MATRIX          = 23,  // Зеленая матрица
    ANIM_EFFECT_MAX             = 24
} custom_anim_effect_t;

typedef struct PACKED {
    uint8_t on_connect_effect;    // Эффект при касании (custom_anim_effect_t)
    uint8_t on_connect_color;     // Цвет (0=Red, 1=Green, 2=Blue, 3=Magenta, 4=Yellow, 5=Cyan, 6=White)
    uint8_t on_connect_speed;     // Скорость: 1 - 10
    uint8_t on_disconnect_effect; // Эффект при убирании карты
    uint8_t on_disconnect_color;  // Цвет при убирании карты
    uint8_t on_disconnect_speed;  // Скорость при убирании карты
    uint8_t idle_effect;          // Фоновый эффект в покое
    uint8_t idle_color;           // Фоновый цвет
    uint8_t idle_speed;           // Фоновая скорость
    uint8_t brightness;           // Яркость: 10 - 99
} custom_anim_config_t;

#endif // CUSTOM_CMD_H
