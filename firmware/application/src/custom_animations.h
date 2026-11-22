#ifndef CUSTOM_ANIMATIONS_H
#define CUSTOM_ANIMATIONS_H

#include <stdint.h>
#include <stdbool.h>
#include "custom_cmd.h"

void custom_anim_init(void);
void custom_anim_get_config(custom_anim_config_t *cfg);
void custom_anim_set_config(const custom_anim_config_t *cfg);
bool custom_anim_save_to_flash(void);
void custom_anim_load_from_flash(void);
void custom_anim_play_connect(void);
void custom_anim_play_disconnect(void);
void custom_anim_idle_step(void);
void custom_anim_play_test(uint8_t effect, uint8_t color, uint8_t speed);
void custom_anim_stop(void);

#endif
