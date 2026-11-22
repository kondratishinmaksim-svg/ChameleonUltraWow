#include "custom_cmd.h"
#include "custom_animations.h"
#include "app_status.h"
#include "dataframe.h"
#include <string.h>

data_frame_tx_t *cmd_processor_get_custom_anim_config(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    (void)status;
    (void)length;
    (void)data;
    custom_anim_config_t config;
    custom_anim_get_config(&config);
    return data_frame_make(cmd, STATUS_SUCCESS, sizeof(custom_anim_config_t), (uint8_t *)&config);
}

data_frame_tx_t *cmd_processor_set_custom_anim_config(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    (void)status;
    if (length < 10 || data == NULL) {
        return data_frame_make(cmd, STATUS_PAR_ERR, 0, NULL);
    }

    custom_anim_config_t new_config;
    memset(&new_config, 0, sizeof(new_config));
    new_config.on_connect_effect    = data[0];
    new_config.on_connect_color     = data[1];
    new_config.on_connect_speed     = data[2];
    new_config.on_disconnect_effect = data[3];
    new_config.on_disconnect_color  = data[4];
    new_config.on_disconnect_speed  = data[5];
    new_config.idle_effect          = data[6];
    new_config.idle_color           = data[7];
    new_config.idle_speed           = data[8];
    new_config.brightness           = data[9];

    if (new_config.brightness > 99) new_config.brightness = 99;
    if (new_config.brightness < 10) new_config.brightness = 10;
    if (new_config.on_connect_speed == 0) new_config.on_connect_speed = 5;
    if (new_config.on_disconnect_speed == 0) new_config.on_disconnect_speed = 5;
    if (new_config.idle_speed == 0) new_config.idle_speed = 5;

    custom_anim_set_config(&new_config);
    return data_frame_make(cmd, STATUS_SUCCESS, 0, NULL);
}

data_frame_tx_t *cmd_processor_trigger_test_anim(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    (void)status;
    if (length < 3 || data == NULL) {
        return data_frame_make(cmd, STATUS_PAR_ERR, 0, NULL);
    }
    custom_anim_play_test(data[0], data[1], data[2]);
    return data_frame_make(cmd, STATUS_SUCCESS, 0, NULL);
}
