/**
 * app_cmd_patch.c
 * Handler extensions for Chameleon Ultra Custom LED Animation Commands
 */

#include "custom_cmd.h"
#include "custom_animations.h"
#include "app_status.h"
#include "dataframe.h"
#include <string.h>

// Command 1100: Get custom animation configuration (10 bytes)
data_frame_tx_t *cmd_processor_get_custom_anim_config(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    (void)status;
    (void)length;
    (void)data;
    
    custom_anim_config_t config;
    custom_anim_get_config(&config);
    return data_frame_make(cmd, STATUS_SUCCESS, sizeof(custom_anim_config_t), (uint8_t *)&config);
}

// Command 1101: Set custom animation configuration (10 bytes)
data_frame_tx_t *cmd_processor_set_custom_anim_config(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    (void)status;
    if (length != sizeof(custom_anim_config_t) || data == NULL) {
        return data_frame_make(cmd, STATUS_PAR_ERR, 0, NULL);
    }

    custom_anim_config_t *new_config = (custom_anim_config_t *)data;
    
    // Validate brightness & speed ranges
    if (new_config->brightness > 99) new_config->brightness = 99;
    if (new_config->on_connect_speed == 0) new_config->on_connect_speed = 5;
    if (new_config->on_disconnect_speed == 0) new_config->on_disconnect_speed = 5;
    if (new_config->idle_speed == 0) new_config->idle_speed = 5;

    custom_anim_set_config(new_config);
    return data_frame_make(cmd, STATUS_SUCCESS, 0, NULL);
}

// Command 1102: Trigger test animation directly [effect, color, speed]
data_frame_tx_t *cmd_processor_trigger_test_anim(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    (void)status;
    if (length < 3 || data == NULL) {
        return data_frame_make(cmd, STATUS_PAR_ERR, 0, NULL);
    }

    uint8_t effect = data[0];
    uint8_t color  = data[1];
    uint8_t speed  = data[2];

    custom_anim_play_test(effect, color, speed);
    return data_frame_make(cmd, STATUS_SUCCESS, 0, NULL);
}
