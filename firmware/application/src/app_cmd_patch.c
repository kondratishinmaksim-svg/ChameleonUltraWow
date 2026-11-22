/**
 * app_cmd_patch.c
 * Handler extensions for Chameleon Ultra Custom LED Animation Commands
 */

#include "custom_cmd.h"
#include "custom_animations.h"
#include "app_status.h"
#include "dataframe.h"
#include "hw_connect.h"
#include "bsp_delay.h"
#include <string.h>

// Command 1100: Get custom animation configuration
data_frame_tx_t *cmd_processor_get_custom_anim_config(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    (void)status;
    (void)length;
    (void)data;
    
    custom_anim_config_t config;
    custom_anim_get_config(&config);
    return data_frame_make(cmd, STATUS_SUCCESS, sizeof(custom_anim_config_t), (uint8_t *)&config);
}

// Command 1101: Set custom animation configuration
data_frame_tx_t *cmd_processor_set_custom_anim_config(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    (void)status;
    if (length < 9 || data == NULL) {
        return data_frame_make(cmd, STATUS_PAR_ERR, 0, NULL);
    }

    custom_anim_config_t new_config;
    custom_anim_get_config(&new_config);
    
    new_config.on_connect_effect    = data[0];
    new_config.on_connect_color     = data[1];
    new_config.on_connect_speed     = data[2];
    new_config.on_disconnect_effect = data[3];
    new_config.on_disconnect_color  = data[4];
    new_config.on_disconnect_speed  = data[5];
    new_config.idle_effect          = data[6];
    new_config.idle_color           = data[7];
    new_config.idle_speed           = data[8];

    if (length >= 12) {
        new_config.poweron_effect   = data[9];
        new_config.poweroff_effect  = data[10];
        new_config.power_color      = data[11];
    }

    // Apply and immediately save to Flash FDS
    custom_anim_set_config(&new_config);

    // Подтверждение на физических диодах: двойная зеленая вспышка!
    uint32_t *led_pins = hw_get_led_array();
    set_slot_light_color(RGB_GREEN);
    for (uint8_t i = 0; i < RGB_LIST_NUM; i++) nrf_gpio_pin_set(led_pins[i]);
    bsp_delay_ms(80);
    for (uint8_t i = 0; i < RGB_LIST_NUM; i++) nrf_gpio_pin_clear(led_pins[i]);
    bsp_delay_ms(60);
    for (uint8_t i = 0; i < RGB_LIST_NUM; i++) nrf_gpio_pin_set(led_pins[i]);
    bsp_delay_ms(80);
    for (uint8_t i = 0; i < RGB_LIST_NUM; i++) nrf_gpio_pin_clear(led_pins[i]);

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
