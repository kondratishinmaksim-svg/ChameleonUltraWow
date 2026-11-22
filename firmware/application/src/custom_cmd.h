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
    ANIM_EFFECT_CYBERPUNK       = 24,  // Неоновый киберпанк (Magenta + Cyan)
    ANIM_EFFECT_BINARY_RADAR    = 25,  // Радар: лучи сходятся в центр
    ANIM_EFFECT_AURORA          = 26,  // Северное сияние (Green -> Cyan -> Blue)
    ANIM_EFFECT_CHAOS_GLITCH    = 27,  // Хакерский глитч: рваные вспышки
    ANIM_EFFECT_HYPERSPACE      = 28,  // Разгон в гиперпространство
    ANIM_EFFECT_COMET_TAIL      = 29,  // Комета с затухающим хвостом
    ANIM_EFFECT_MAX             = 30
} custom_anim_effect_t;

// Ровно 12 байт (3 слова FDS) — никаких сбросов после перезагрузки
typedef struct PACKED {
    uint8_t on_connect_effect;    // 0: Касание карты
    uint8_t on_connect_color;     // 1: Цвет касания
    uint8_t on_connect_speed;     // 2: Скорость касания
    uint8_t on_disconnect_effect; // 3: Убирание карты
    uint8_t on_disconnect_color;  // 4: Цвет убирания
    uint8_t on_disconnect_speed;  // 5: Скорость убирания
    uint8_t idle_effect;          // 6: В покое (фон)
    uint8_t idle_color;           // 7: Цвет в покое
    uint8_t idle_speed;           // 8: Скорость в покое
    uint8_t poweron_effect;       // 9: При включении (Boot / WakeUp)
    uint8_t poweroff_effect;      // 10: При выключении (Sleep / PowerOff)
    uint8_t power_color;          // 11: Цвет анимации включения/выключения
} custom_anim_config_t;

#endif // CUSTOM_CMD_H
