#ifndef CUSTOM_CMD_H
#define CUSTOM_CMD_H

#include <stdint.h>
#include "utils.h"

#define DATA_CMD_GET_CUSTOM_ANIM_CONFIG         (1100)
#define DATA_CMD_SET_CUSTOM_ANIM_CONFIG         (1101)
#define DATA_CMD_TRIGGER_TEST_ANIM              (1102)

typedef enum {
    ANIM_EFFECT_NONE            = 0,
    ANIM_EFFECT_PULSE_WHITE     = 10,
    ANIM_EFFECT_PULSE_COLOR     = 11,
    ANIM_EFFECT_SWEEP_WAVE      = 12,
    ANIM_EFFECT_BREATHE         = 13,
    ANIM_EFFECT_RAINBOW         = 14,
    ANIM_EFFECT_SLOT_CHASE      = 15,
    ANIM_EFFECT_SPARKLE         = 16,
    ANIM_EFFECT_SOLID           = 17,
    ANIM_EFFECT_SYMMETRIC_BURST = 18,
    ANIM_EFFECT_STROBE          = 19,
    ANIM_EFFECT_HEARTBEAT       = 20,
    ANIM_EFFECT_POLICE          = 21,
    ANIM_EFFECT_FIRE            = 22,
    ANIM_EFFECT_MATRIX          = 23,
    ANIM_EFFECT_MAX             = 24
} custom_anim_effect_t;

typedef struct PACKED {
    uint8_t on_connect_effect;
    uint8_t on_connect_color;
    uint8_t on_connect_speed;
    uint8_t on_disconnect_effect;
    uint8_t on_disconnect_color;
    uint8_t on_disconnect_speed;
    uint8_t idle_effect;
    uint8_t idle_color;
    uint8_t idle_speed;
    uint8_t brightness;
    uint8_t magic1;
    uint8_t magic2;
} custom_anim_config_t;

#endif
