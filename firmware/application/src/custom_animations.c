#include "custom_animations.h"
#include "hw_connect.h"
#include "rgb_marquee.h"
#include "bsp_delay.h"
#include "bsp_time.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "fds_util.h"
#include <string.h>
#include <stdlib.h>

#define FDS_CUSTOM_ANIM_FILE_ID    0x1005
#define FDS_CUSTOM_ANIM_RECORD_KEY 0x0001

extern bool g_is_tag_emulating;

static custom_anim_config_t m_config = {
    .on_connect_effect    = ANIM_EFFECT_PULSE_WHITE,
    .on_connect_color     = RGB_WHITE,
    .on_connect_speed     = 7,
    .on_disconnect_effect = ANIM_EFFECT_NONE,
    .on_disconnect_color  = RGB_WHITE,
    .on_disconnect_speed  = 5,
    .idle_effect          = ANIM_EFFECT_SLOT_CHASE,
    .idle_color           = RGB_CYAN,
    .idle_speed           = 5,
    .poweron_effect       = ANIM_EFFECT_HYPERSPACE,
    .poweroff_effect      = ANIM_EFFECT_SYMMETRIC_BURST,
    .power_color          = RGB_CYAN
};

static autotimer *m_idle_timer = NULL;
static bool m_was_emulating = false;
static uint16_t m_idle_step_counter = 0;
static int8_t m_chase_dir = 1;
static int8_t m_chase_pos = 0;

static void all_leds_off(void) {
    uint32_t *led_pins = hw_get_led_array();
    for (uint8_t i = 0; i < RGB_LIST_NUM; i++) nrf_gpio_pin_clear(led_pins[i]);
}

static void all_leds_on(void) {
    uint32_t *led_pins = hw_get_led_array();
    for (uint8_t i = 0; i < RGB_LIST_NUM; i++) nrf_gpio_pin_set(led_pins[i]);
}

bool custom_anim_save_to_flash(void) {
    uint8_t buf[12];
    buf[0]  = m_config.on_connect_effect;
    buf[1]  = m_config.on_connect_color;
    buf[2]  = m_config.on_connect_speed;
    buf[3]  = m_config.on_disconnect_effect;
    buf[4]  = m_config.on_disconnect_color;
    buf[5]  = m_config.on_disconnect_speed;
    buf[6]  = m_config.idle_effect;
    buf[7]  = m_config.idle_color;
    buf[8]  = m_config.idle_speed;
    buf[9]  = m_config.poweron_effect;
    buf[10] = m_config.poweroff_effect;
    buf[11] = m_config.power_color;
    return fds_write_sync(FDS_CUSTOM_ANIM_FILE_ID, FDS_CUSTOM_ANIM_RECORD_KEY, 12, buf);
}

void custom_anim_load_from_flash(void) {
    uint8_t buf[16] = {0};
    uint16_t len = sizeof(buf);
    if (fds_read_sync(FDS_CUSTOM_ANIM_FILE_ID, FDS_CUSTOM_ANIM_RECORD_KEY, &len, buf)) {
        if (len >= 12) {
            m_config.on_connect_effect    = buf[0];
            m_config.on_connect_color     = buf[1];
            m_config.on_connect_speed     = buf[2];
            m_config.on_disconnect_effect = buf[3];
            m_config.on_disconnect_color  = buf[4];
            m_config.on_disconnect_speed  = buf[5];
            m_config.idle_effect          = buf[6];
            m_config.idle_color           = buf[7];
            m_config.idle_speed           = buf[8];
            m_config.poweron_effect       = buf[9];
            m_config.poweroff_effect      = buf[10];
            m_config.power_color          = buf[11];
            return;
        }
    }
    custom_anim_save_to_flash();
}

void custom_anim_init(void) {
    m_idle_timer = bsp_obtain_timer(0);
    m_was_emulating = false;
    m_idle_step_counter = 0;
    m_chase_dir = 1;
    m_chase_pos = 0;
    custom_anim_load_from_flash();
}

void custom_anim_get_config(custom_anim_config_t *cfg) {
    if (cfg) memcpy(cfg, &m_config, sizeof(custom_anim_config_t));
}

void custom_anim_set_config(const custom_anim_config_t *cfg) {
    if (cfg) {
        m_config.on_connect_effect    = cfg->on_connect_effect;
        m_config.on_connect_color     = cfg->on_connect_color;
        m_config.on_connect_speed     = cfg->on_connect_speed;
        m_config.on_disconnect_effect = cfg->on_disconnect_effect;
        m_config.on_disconnect_color  = cfg->on_disconnect_color;
        m_config.on_disconnect_speed  = cfg->on_disconnect_speed;
        m_config.idle_effect          = cfg->idle_effect;
        m_config.idle_color           = cfg->idle_color;
        m_config.idle_speed           = cfg->idle_speed;
        m_config.poweron_effect       = cfg->poweron_effect;
        m_config.poweroff_effect      = cfg->poweroff_effect;
        m_config.power_color          = cfg->power_color;
        all_leds_off();
        m_idle_step_counter = 0;
        custom_anim_save_to_flash();
    }
}

void custom_anim_stop(void) {
    all_leds_off();
    rgb_marquee_reset();
}

/* 1. ПУЛЬСАЦИЯ */
static void run_pulse_animation(uint8_t color, uint8_t speed) {
    uint32_t *led_pins = hw_get_led_array();
    all_leds_off();
    set_slot_light_color((chameleon_rgb_type_t)color);
    uint16_t cycle_us = 1000;
    uint8_t step_inc = (speed >= 8) ? 4 : ((speed >= 5) ? 3 : 2);

    for (uint8_t duty = 0; duty <= 95; duty += step_inc) {
        for (uint8_t r = 0; r < 3; r++) {
            if (duty > 0) {
                for (uint8_t i = 0; i < RGB_LIST_NUM; i++) nrf_gpio_pin_set(led_pins[i]);
                bsp_delay_us((duty * cycle_us) / 100);
            }
            if (duty < 100) {
                for (uint8_t i = 0; i < RGB_LIST_NUM; i++) nrf_gpio_pin_clear(led_pins[i]);
                bsp_delay_us(((100 - duty) * cycle_us) / 100);
            }
        }
    }
    bsp_delay_ms(35);
    for (int16_t duty = 95; duty >= 0; duty -= step_inc) {
        for (uint8_t r = 0; r < 3; r++) {
            if (duty > 0) {
                for (uint8_t i = 0; i < RGB_LIST_NUM; i++) nrf_gpio_pin_set(led_pins[i]);
                bsp_delay_us((duty * cycle_us) / 100);
            }
            if (duty < 100) {
                for (uint8_t i = 0; i < RGB_LIST_NUM; i++) nrf_gpio_pin_clear(led_pins[i]);
                bsp_delay_us(((100 - duty) * cycle_us) / 100);
            }
        }
    }
    all_leds_off();
}

/* 2. БЕГУЩАЯ ВОЛНА */
static void run_wave_animation(uint8_t color, uint8_t speed) {
    uint32_t *led_pins = hw_get_led_array();
    all_leds_off();
    set_slot_light_color((chameleon_rgb_type_t)color);
    uint16_t delay = 100 - (speed * 8);
    if (delay < 15) delay = 15;

    for (int8_t i = 0; i < (int8_t)RGB_LIST_NUM; i++) {
        all_leds_off();
        nrf_gpio_pin_set(led_pins[i]);
        if (i > 0) nrf_gpio_pin_set(led_pins[i - 1]);
        bsp_delay_ms(delay);
    }
    for (int8_t i = (int8_t)RGB_LIST_NUM - 1; i >= 0; i--) {
        all_leds_off();
        nrf_gpio_pin_set(led_pins[i]);
        if (i < (int8_t)RGB_LIST_NUM - 1) nrf_gpio_pin_set(led_pins[i + 1]);
        bsp_delay_ms(delay);
    }
    all_leds_off();
}

/* 3. РАДУГА */
static void run_rainbow_animation(uint8_t speed) {
    chameleon_rgb_type_t colors[] = { RGB_RED, RGB_YELLOW, RGB_GREEN, RGB_CYAN, RGB_BLUE, RGB_MAGENTA, RGB_WHITE };
    uint16_t delay = 180 - (speed * 14);
    if (delay < 30) delay = 30;
    all_leds_on();
    for (uint8_t c = 0; c < 7; c++) {
        set_slot_light_color(colors[c]);
        bsp_delay_ms(delay);
    }
    all_leds_off();
}

/* 4. ВСПЫШКА ИЗ ЦЕНТРА */
static void run_burst_animation(uint8_t color, uint8_t speed) {
    uint32_t *led_pins = hw_get_led_array();
    set_slot_light_color((chameleon_rgb_type_t)color);
    uint16_t delay = 70 - (speed * 5);
    if (delay < 15) delay = 15;
    for (uint8_t step = 0; step < 4; step++) {
        all_leds_off();
        nrf_gpio_pin_set(led_pins[3 - step]);
        nrf_gpio_pin_set(led_pins[4 + step]);
        bsp_delay_ms(delay);
    }
    for (int8_t step = 2; step >= 0; step--) {
        all_leds_off();
        nrf_gpio_pin_set(led_pins[3 - step]);
        nrf_gpio_pin_set(led_pins[4 + step]);
        bsp_delay_ms(delay);
    }
    all_leds_off();
}

/* 5. СТРОБОСКОП */
static void run_strobe_animation(uint8_t color, uint8_t speed) {
    set_slot_light_color((chameleon_rgb_type_t)color);
    uint16_t off_ms = 40 - (speed * 2);
    if (off_ms < 15) off_ms = 15;
    for (uint8_t i = 0; i < 4; i++) {
        all_leds_on();
        bsp_delay_ms(25);
        all_leds_off();
        bsp_delay_ms(off_ms);
    }
}

/* 6. СЕРДЦЕБИЕНИЕ */
static void run_heartbeat_animation(uint8_t color, uint8_t speed) {
    (void)speed;
    set_slot_light_color((chameleon_rgb_type_t)color);
    all_leds_on();
    bsp_delay_ms(40);
    all_leds_off();
    bsp_delay_ms(80);
    all_leds_on();
    bsp_delay_ms(60);
    all_leds_off();
}

/* 7. ПОЛИЦИЯ */
static void run_police_animation(uint8_t speed) {
    uint32_t *led_pins = hw_get_led_array();
    uint16_t delay = 45 - (speed * 2);
    if (delay < 15) delay = 15;
    for (uint8_t cycle = 0; cycle < 3; cycle++) {
        set_slot_light_color(RGB_RED);
        all_leds_off();
        for (uint8_t i = 0; i < 4; i++) nrf_gpio_pin_set(led_pins[i]);
        bsp_delay_ms(delay);
        all_leds_off();
        bsp_delay_ms(delay);
        for (uint8_t i = 0; i < 4; i++) nrf_gpio_pin_set(led_pins[i]);
        bsp_delay_ms(delay);
        all_leds_off();
        bsp_delay_ms(delay);

        set_slot_light_color(RGB_BLUE);
        all_leds_off();
        for (uint8_t i = 4; i < 8; i++) nrf_gpio_pin_set(led_pins[i]);
        bsp_delay_ms(delay);
        all_leds_off();
        bsp_delay_ms(delay);
        for (uint8_t i = 4; i < 8; i++) nrf_gpio_pin_set(led_pins[i]);
        bsp_delay_ms(delay);
        all_leds_off();
        bsp_delay_ms(delay);
    }
    all_leds_off();
}

/* 8. ОГОНЬ */
static void run_fire_animation(uint8_t speed) {
    uint32_t *led_pins = hw_get_led_array();
    chameleon_rgb_type_t fire_cols[] = { RGB_RED, RGB_YELLOW, RGB_WHITE };
    uint8_t frames = speed * 4 + 10;
    for (uint8_t f = 0; f < frames; f++) {
        all_leds_off();
        set_slot_light_color(fire_cols[rand() % 3]);
        uint8_t count = 3 + (rand() % 4);
        for (uint8_t c = 0; c < count; c++) nrf_gpio_pin_set(led_pins[rand() % RGB_LIST_NUM]);
        bsp_delay_ms(25 + (rand() % 20));
    }
    all_leds_off();
}

/* 9. МАТРИЦА */
static void run_matrix_animation(uint8_t speed) {
    uint32_t *led_pins = hw_get_led_array();
    set_slot_light_color(RGB_GREEN);
    uint16_t delay = 50 - (speed * 3);
    if (delay < 10) delay = 10;
    for (uint8_t pass = 0; pass < 2; pass++) {
        for (uint8_t i = 0; i < RGB_LIST_NUM; i++) {
            all_leds_off();
            nrf_gpio_pin_set(led_pins[i]);
            if (i > 0) nrf_gpio_pin_set(led_pins[i - 1]);
            bsp_delay_ms(delay);
        }
    }
    all_leds_off();
}

/* 10. КИБЕРПАНК */
static void run_cyberpunk_animation(uint8_t speed) {
    uint32_t *led_pins = hw_get_led_array();
    uint16_t delay = 60 - (speed * 4);
    if (delay < 15) delay = 15;
    for (uint8_t r = 0; r < 3; r++) {
        set_slot_light_color(RGB_MAGENTA);
        all_leds_off();
        for (uint8_t i = 0; i < RGB_LIST_NUM; i += 2) nrf_gpio_pin_set(led_pins[i]);
        bsp_delay_ms(delay);
        set_slot_light_color(RGB_CYAN);
        all_leds_off();
        for (uint8_t i = 1; i < RGB_LIST_NUM; i += 2) nrf_gpio_pin_set(led_pins[i]);
        bsp_delay_ms(delay);
    }
    all_leds_off();
}

/* 11. РАДАР */
static void run_radar_animation(uint8_t color, uint8_t speed) {
    uint32_t *led_pins = hw_get_led_array();
    set_slot_light_color((chameleon_rgb_type_t)color);
    uint16_t delay = 65 - (speed * 4);
    if (delay < 12) delay = 12;
    for (uint8_t p = 0; p < 4; p++) {
        all_leds_off();
        nrf_gpio_pin_set(led_pins[p]);
        nrf_gpio_pin_set(led_pins[7 - p]);
        bsp_delay_ms(delay);
    }
    // Вспышка в центре
    set_slot_light_color(RGB_WHITE);
    all_leds_off();
    nrf_gpio_pin_set(led_pins[3]);
    nrf_gpio_pin_set(led_pins[4]);
    bsp_delay_ms(delay * 2);
    all_leds_off();
}

/* 12. СЕВЕРНОЕ СИЯНИЕ */
static void run_aurora_animation(uint8_t speed) {
    uint32_t *led_pins = hw_get_led_array();
    chameleon_rgb_type_t aurora_cols[] = { RGB_GREEN, RGB_CYAN, RGB_BLUE, RGB_MAGENTA };
    uint16_t delay = 90 - (speed * 6);
    if (delay < 20) delay = 20;
    for (uint8_t c = 0; c < 4; c++) {
        set_slot_light_color(aurora_cols[c]);
        for (uint8_t i = 0; i < RGB_LIST_NUM; i++) {
            all_leds_off();
            nrf_gpio_pin_set(led_pins[i]);
            if (i > 0) nrf_gpio_pin_set(led_pins[i - 1]);
            bsp_delay_ms(delay);
        }
    }
    all_leds_off();
}

/* 13. КИБЕР-ГЛИТЧ */
static void run_glitch_animation(uint8_t speed) {
    uint32_t *led_pins = hw_get_led_array();
    chameleon_rgb_type_t g_cols[] = { RGB_CYAN, RGB_WHITE, RGB_MAGENTA, RGB_RED };
    uint8_t glitches = speed * 3 + 6;
    for (uint8_t g = 0; g < glitches; g++) {
        all_leds_off();
        set_slot_light_color(g_cols[rand() % 4]);
        uint8_t p = rand() % RGB_LIST_NUM;
        nrf_gpio_pin_set(led_pins[p]);
        if (rand() % 2) nrf_gpio_pin_set(led_pins[(p + 3) % RGB_LIST_NUM]);
        bsp_delay_ms(15 + (rand() % 35));
        all_leds_off();
        bsp_delay_ms(10 + (rand() % 25));
    }
    all_leds_off();
}

/* 14. ГИПЕРПРОСТРАНСТВО (Идеально для Power On!) */
static void run_hyperspace_animation(uint8_t color, uint8_t speed) {
    uint32_t *led_pins = hw_get_led_array();
    set_slot_light_color((chameleon_rgb_type_t)color);
    (void)speed;
    uint16_t d = 90;
    for (uint8_t round = 0; round < 3; round++) {
        for (uint8_t p = 0; p < 4; p++) {
            all_leds_off();
            nrf_gpio_pin_set(led_pins[3 - p]);
            nrf_gpio_pin_set(led_pins[4 + p]);
            bsp_delay_ms(d);
        }
        if (d > 20) d -= 25;
    }
    // Финальная яркая вспышка белым
    set_slot_light_color(RGB_WHITE);
    all_leds_on();
    bsp_delay_ms(120);
    all_leds_off();
}

/* 15. КОМЕТА */
static void run_comet_animation(uint8_t color, uint8_t speed) {
    uint32_t *led_pins = hw_get_led_array();
    set_slot_light_color((chameleon_rgb_type_t)color);
    uint16_t delay = 80 - (speed * 6);
    if (delay < 15) delay = 15;
    for (int8_t i = 0; i < (int8_t)RGB_LIST_NUM + 2; i++) {
        all_leds_off();
        if (i < (int8_t)RGB_LIST_NUM) nrf_gpio_pin_set(led_pins[i]);
        if (i > 0 && (i - 1) < (int8_t)RGB_LIST_NUM) nrf_gpio_pin_set(led_pins[i - 1]);
        bsp_delay_ms(delay);
    }
    all_leds_off();
}

static void execute_effect(uint8_t effect, uint8_t color, uint8_t speed) {
    switch (effect) {
        case ANIM_EFFECT_PULSE_WHITE:
            run_pulse_animation(RGB_WHITE, speed);
            break;
        case ANIM_EFFECT_PULSE_COLOR:
        case ANIM_EFFECT_BREATHE:
            run_pulse_animation(color, speed);
            break;
        case ANIM_EFFECT_SWEEP_WAVE:
        case ANIM_EFFECT_SLOT_CHASE:
            run_wave_animation(color, speed);
            break;
        case ANIM_EFFECT_RAINBOW:
            run_rainbow_animation(speed);
            break;
        case ANIM_EFFECT_SYMMETRIC_BURST:
            run_burst_animation(color, speed);
            break;
        case ANIM_EFFECT_STROBE:
            run_strobe_animation(color, speed);
            break;
        case ANIM_EFFECT_HEARTBEAT:
            run_heartbeat_animation(color, speed);
            break;
        case ANIM_EFFECT_POLICE:
            run_police_animation(speed);
            break;
        case ANIM_EFFECT_FIRE:
            run_fire_animation(speed);
            break;
        case ANIM_EFFECT_MATRIX:
            run_matrix_animation(speed);
            break;
        case ANIM_EFFECT_CYBERPUNK:
            run_cyberpunk_animation(speed);
            break;
        case ANIM_EFFECT_BINARY_RADAR:
            run_radar_animation(color, speed);
            break;
        case ANIM_EFFECT_AURORA:
            run_aurora_animation(speed);
            break;
        case ANIM_EFFECT_CHAOS_GLITCH:
            run_glitch_animation(speed);
            break;
        case ANIM_EFFECT_HYPERSPACE:
            run_hyperspace_animation(color, speed);
            break;
        case ANIM_EFFECT_COMET_TAIL:
            run_comet_animation(color, speed);
            break;
        case ANIM_EFFECT_SOLID:
            set_slot_light_color((chameleon_rgb_type_t)color);
            all_leds_on();
            bsp_delay_ms(300);
            all_leds_off();
            break;
        case ANIM_EFFECT_NONE:
        default:
            all_leds_off();
            break;
    }
}

void custom_anim_play_connect(void) {
    execute_effect(m_config.on_connect_effect, m_config.on_connect_color, m_config.on_connect_speed);
}

void custom_anim_play_disconnect(void) {
    execute_effect(m_config.on_disconnect_effect, m_config.on_disconnect_color, m_config.on_disconnect_speed);
}

bool custom_anim_play_poweron(void) {
    if (m_config.poweron_effect == ANIM_EFFECT_NONE) return false;
    all_leds_off();
    execute_effect(m_config.poweron_effect, m_config.power_color, 7);
    return true;
}

bool custom_anim_play_poweroff(void) {
    if (m_config.poweroff_effect == ANIM_EFFECT_NONE) return false;
    all_leds_off();
    execute_effect(m_config.poweroff_effect, m_config.power_color, 6);
    all_leds_off();
    return true;
}

void custom_anim_play_test(uint8_t effect, uint8_t color, uint8_t speed) {
    execute_effect(effect, color, speed);
}

void custom_anim_idle_step(void) {
    // 1. ДЕТЕКЦИЯ КАРТЫ
    if (g_is_tag_emulating && !m_was_emulating) {
        m_was_emulating = true;
        all_leds_off();
        custom_anim_play_connect();
        return;
    } else if (!g_is_tag_emulating && m_was_emulating) {
        m_was_emulating = false;
        all_leds_off();
        custom_anim_play_disconnect();
        return;
    }

    if (g_is_tag_emulating) return;

    // 2. ФОНОВЫЙ РЕЖИМ
    if (m_config.idle_effect == ANIM_EFFECT_NONE) {
        all_leds_off();
        return;
    }

    if (m_idle_timer == NULL) {
        m_idle_timer = bsp_obtain_timer(0);
        bsp_set_timer(m_idle_timer, 0);
    }

    uint16_t delay_ms = 120 - (m_config.idle_speed * 10);
    if (delay_ms < 20) delay_ms = 20;

    if (!NO_TIMEOUT_1MS(m_idle_timer, delay_ms)) {
        bsp_set_timer(m_idle_timer, 0);
        uint32_t *led_pins = hw_get_led_array();
        set_slot_light_color((chameleon_rgb_type_t)m_config.idle_color);

        switch (m_config.idle_effect) {
            case ANIM_EFFECT_SLOT_CHASE: {
                all_leds_off();
                nrf_gpio_pin_set(led_pins[m_chase_pos]);
                m_chase_pos += m_chase_dir;
                if (m_chase_pos >= (int8_t)RGB_LIST_NUM - 1) { m_chase_pos = RGB_LIST_NUM - 1; m_chase_dir = -1; }
                else if (m_chase_pos <= 0) { m_chase_pos = 0; m_chase_dir = 1; }
                break;
            }
            case ANIM_EFFECT_SWEEP_WAVE: {
                all_leds_off();
                nrf_gpio_pin_set(led_pins[m_chase_pos]);
                if (m_chase_pos > 0) nrf_gpio_pin_set(led_pins[m_chase_pos - 1]);
                m_chase_pos += m_chase_dir;
                if (m_chase_pos >= (int8_t)RGB_LIST_NUM - 1) { m_chase_pos = RGB_LIST_NUM - 1; m_chase_dir = -1; }
                else if (m_chase_pos <= 0) { m_chase_pos = 0; m_chase_dir = 1; }
                break;
            }
            case ANIM_EFFECT_BREATHE:
            case ANIM_EFFECT_PULSE_COLOR:
            case ANIM_EFFECT_PULSE_WHITE: {
                m_idle_step_counter++;
                if ((m_idle_step_counter % 12) < 6) all_leds_on();
                else all_leds_off();
                break;
            }
            case ANIM_EFFECT_RAINBOW: {
                chameleon_rgb_type_t cols[] = { RGB_RED, RGB_YELLOW, RGB_GREEN, RGB_CYAN, RGB_BLUE, RGB_MAGENTA, RGB_WHITE };
                all_leds_on();
                set_slot_light_color(cols[(m_idle_step_counter++) % 7]);
                break;
            }
            case ANIM_EFFECT_POLICE: {
                m_idle_step_counter++;
                all_leds_off();
                if ((m_idle_step_counter % 4) < 2) {
                    set_slot_light_color(RGB_RED);
                    for (uint8_t i = 0; i < 4; i++) nrf_gpio_pin_set(led_pins[i]);
                } else {
                    set_slot_light_color(RGB_BLUE);
                    for (uint8_t i = 4; i < 8; i++) nrf_gpio_pin_set(led_pins[i]);
                }
                break;
            }
            case ANIM_EFFECT_FIRE:
            case ANIM_EFFECT_SPARKLE: {
                all_leds_off();
                if (m_config.idle_effect == ANIM_EFFECT_FIRE) {
                    chameleon_rgb_type_t fc[] = { RGB_RED, RGB_YELLOW, RGB_WHITE };
                    set_slot_light_color(fc[rand() % 3]);
                }
                nrf_gpio_pin_set(led_pins[rand() % RGB_LIST_NUM]);
                break;
            }
            case ANIM_EFFECT_MATRIX: {
                all_leds_off();
                set_slot_light_color(RGB_GREEN);
                uint8_t p = (m_idle_step_counter++) % RGB_LIST_NUM;
                nrf_gpio_pin_set(led_pins[p]);
                if (p > 0) nrf_gpio_pin_set(led_pins[p - 1]);
                break;
            }
            case ANIM_EFFECT_CYBERPUNK: {
                m_idle_step_counter++;
                all_leds_off();
                if ((m_idle_step_counter % 4) < 2) {
                    set_slot_light_color(RGB_MAGENTA);
                    for (uint8_t i = 0; i < 8; i += 2) nrf_gpio_pin_set(led_pins[i]);
                } else {
                    set_slot_light_color(RGB_CYAN);
                    for (uint8_t i = 1; i < 8; i += 2) nrf_gpio_pin_set(led_pins[i]);
                }
                break;
            }
            case ANIM_EFFECT_BINARY_RADAR: {
                all_leds_off();
                uint8_t p = (m_idle_step_counter++) % 4;
                nrf_gpio_pin_set(led_pins[p]);
                nrf_gpio_pin_set(led_pins[7 - p]);
                break;
            }
            case ANIM_EFFECT_AURORA: {
                chameleon_rgb_type_t a_cols[] = { RGB_GREEN, RGB_CYAN, RGB_BLUE, RGB_MAGENTA };
                all_leds_off();
                set_slot_light_color(a_cols[(m_idle_step_counter / 8) % 4]);
                uint8_t p = (m_idle_step_counter++) % RGB_LIST_NUM;
                nrf_gpio_pin_set(led_pins[p]);
                if (p > 0) nrf_gpio_pin_set(led_pins[p - 1]);
                break;
            }
            case ANIM_EFFECT_CHAOS_GLITCH: {
                all_leds_off();
                if (rand() % 3 == 0) {
                    chameleon_rgb_type_t g_cols[] = { RGB_CYAN, RGB_WHITE, RGB_MAGENTA };
                    set_slot_light_color(g_cols[rand() % 3]);
                    nrf_gpio_pin_set(led_pins[rand() % RGB_LIST_NUM]);
                }
                break;
            }
            case ANIM_EFFECT_HYPERSPACE: {
                all_leds_off();
                uint8_t p = (m_idle_step_counter++) % 4;
                nrf_gpio_pin_set(led_pins[3 - p]);
                nrf_gpio_pin_set(led_pins[4 + p]);
                break;
            }
            case ANIM_EFFECT_COMET_TAIL: {
                all_leds_off();
                uint8_t p = (m_idle_step_counter++) % (RGB_LIST_NUM + 2);
                if (p < RGB_LIST_NUM) nrf_gpio_pin_set(led_pins[p]);
                if (p > 0 && (p - 1) < RGB_LIST_NUM) nrf_gpio_pin_set(led_pins[p - 1]);
                break;
            }
            case ANIM_EFFECT_HEARTBEAT: {
                m_idle_step_counter++;
                uint8_t hb = m_idle_step_counter % 10;
                if (hb == 0 || hb == 2) all_leds_on();
                else all_leds_off();
                break;
            }
            case ANIM_EFFECT_STROBE: {
                m_idle_step_counter++;
                if ((m_idle_step_counter % 2) == 0) all_leds_on();
                else all_leds_off();
                break;
            }
            case ANIM_EFFECT_SYMMETRIC_BURST: {
                all_leds_off();
                uint8_t p = (m_idle_step_counter++) % 4;
                nrf_gpio_pin_set(led_pins[3 - p]);
                nrf_gpio_pin_set(led_pins[4 + p]);
                break;
            }
            case ANIM_EFFECT_SOLID: {
                all_leds_on();
                break;
            }
            default: break;
        }
    }
}
