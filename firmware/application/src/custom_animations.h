#ifndef CUSTOM_ANIMATIONS_H
#define CUSTOM_ANIMATIONS_H

#include <stdint.h>
#include <stdbool.h>
#include "custom_cmd.h"

// Initialize custom animation subsystem
void custom_anim_init(void);

// Get current configuration
void custom_anim_get_config(custom_anim_config_t *cfg);

// Set and save new configuration
void custom_anim_set_config(const custom_anim_config_t *cfg);

// Trigger on-connect / card touch animation (e.g. pulse white)
void custom_anim_play_connect(void);

// Trigger on-disconnect animation
void custom_anim_play_disconnect(void);

// Run non-blocking step for idle animation
void custom_anim_idle_step(void);

// Play a specific test animation directly
void custom_anim_play_test(uint8_t effect, uint8_t color, uint8_t speed);

// Stop any running custom animation and restore default LED state
void custom_anim_stop(void);

#endif // CUSTOM_ANIMATIONS_H
