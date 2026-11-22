#include "custom_animations.h"
#include "hw_connect.h"
#include "nrf_gpio.h"
#include "bsp_delay.h"
#include "bsp_time.h"
#include "rgb_marquee.h"
#include <stdlib.h>
#include <string.h>

#define NRF_LOG_MODULE_NAME custom_anim
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
NRF_LOG_MODULE_REGISTER();

// Default animation configuration
static custom_anim_config_t m_config = {
    .on_connect_effect    = ANIM_EFFECT_PULSE_WHITE, // По умолчанию: белая пульсация при касании
    .on_connect_color     = RGB_WHITE,
    .on_connect_speed     = 7,
    .on_disconnect_effect = ANIM_EFFECT_SWEEP_WAVE,
    .on_disconnect_color  = RGB_CYAN,
    .on_disconnect_speed  = 6,
    .idle_effect          = ANIM_EFFECT_BREATHE,
    .idle_color           = RGB_BLUE,
    .idle_speed           = 4,
    .brightness           = 90
};

static autotimer *m_idle_timer = NULL;
static uint8_t m_idle_step = 0;
static int16_t m_breathe_val = 0;
static int8_t m_breathe_dir = 1;

void custom_anim_init(void) {
    m_idle_timer = bsp_obtain_timer(0);
    NRF_LOG_INFO("Custom animations initialized.");
}

void custom_anim_get_config(custom_anim_config_t *cfg) {
    if (cfg) {
        memcpy(cfg, &m_config, sizeof(custom_anim_config_t));
    }
}

void custom_anim_set_config(const custom_anim_config_t *cfg) {
    if (cfg) {
        memcpy(&m_config, cfg, sizeof(custom_anim_config_t));
        // Reset idle step on config change
        m_idle_step = 0;
        NRF_LOG_INFO("Custom animation config updated.");
    }
}

static void all_leds_off(void) {
    uint32_t *led_pins = hw_get_led_array();
    for (uint8_t i = 0; i < RGB_LIST_NUM; i++) {
        nrf_gpio_pin_clear(led_pins[i]);
    }
}

static void all_leds_on(void) {
    uint32_t *led_pins = hw_get_led_array();
    for (uint8_t i = 0; i < RGB_LIST_NUM; i++) {
        nrf_gpio_pin_set(led_pins[i]);
    }
}

// Software PWM pulse for smooth breathing / pulsing across all LEDs
static void effect_pulse(uint8_t color, uint8_t speed, uint8_t max_brightness) {
    uint32_t *led_pins = hw_get_led_array();
    set_slot_light_color((chameleon_rgb_type_t)color);
    
    uint16_t step_delay_us = (11 - (speed > 10 ? 10 : (speed < 1 ? 1 : speed))) * 120;
    uint8_t peak = (max_brightness > 99) ? 99 : (max_brightness < 10 ? 10 : max_brightness);

    // Fade IN
    for (uint8_t b = 1; b <= peak; b += 2) {
        for (uint8_t cycle = 0; cycle < 5; cycle++) {
            // Turn ON
            for (uint8_t i = 0; i < RGB_LIST_NUM; i++) {
                nrf_gpio_pin_set(led_pins[i]);
            }
            bsp_delay_us(b * 15);
            // Turn OFF
            for (uint8_t i = 0; i < RGB_LIST_NUM; i++) {
                nrf_gpio_pin_clear(led_pins[i]);
            }
            bsp_delay_us((100 - b) * 15);
        }
        bsp_delay_us(step_delay_us);
    }

    bsp_delay_ms(30);

    // Fade OUT
    for (int16_t b = peak; b >= 0; b -= 2) {
        for (uint8_t cycle = 0; cycle < 5; cycle++) {
            for (uint8_t i = 0; i < RGB_LIST_NUM; i++) {
                nrf_gpio_pin_set(led_pins[i]);
            }
            bsp_delay_us(b * 15);
            for (uint8_t i = 0; i < RGB_LIST_NUM; i++) {
                nrf_gpio_pin_clear(led_pins[i]);
            }
            bsp_delay_us((100 - b) * 15);
        }
        bsp_delay_us(step_delay_us);
    }

    all_leds_off();
}

// Running wave back and forth
static void effect_sweep_wave(uint8_t color, uint8_t speed) {
    uint32_t *led_pins = hw_get_led_array();
    set_slot_light_color((chameleon_rgb_type_t)color);
    uint16_t delay = 120 - (speed * 10);
    if (delay < 20) delay = 20;

    // Left to Right
    for (int8_t i = 0; i < (int8_t)RGB_LIST_NUM; i++) {
        all_leds_off();
        nrf_gpio_pin_set(led_pins[i]);
        if (i > 0) nrf_gpio_pin_set(led_pins[i - 1]);
        bsp_delay_ms(delay);
    }
    // Right to Left
    for (int8_t i = (int8_t)RGB_LIST_NUM - 1; i >= 0; i--) {
        all_leds_off();
        nrf_gpio_pin_set(led_pins[i]);
        if (i < (int8_t)RGB_LIST_NUM - 1) nrf_gpio_pin_set(led_pins[i + 1]);
        bsp_delay_ms(delay);
    }
    all_leds_off();
}

// Rainbow cycle through distinct colors
static void effect_rainbow(uint8_t speed) {
    chameleon_rgb_type_t rainbow_colors[] = {
        RGB_RED, RGB_YELLOW, RGB_GREEN, RGB_CYAN, RGB_BLUE, RGB_MAGENTA, RGB_WHITE
    };
    uint8_t count = sizeof(rainbow_colors) / sizeof(rainbow_colors[0]);
    uint16_t delay = 200 - (speed * 15);
    if (delay < 40) delay = 40;

    all_leds_on();
    for (uint8_t c = 0; c < count; c++) {
        set_slot_light_color(rainbow_colors[c]);
        bsp_delay_ms(delay);
    }
    all_leds_off();
}

// Slot Chase: Knight Rider style dot
static void effect_slot_chase(uint8_t color, uint8_t speed) {
    uint32_t *led_pins = hw_get_led_array();
    set_slot_light_color((chameleon_rgb_type_t)color);
    uint16_t delay = 90 - (speed * 7);
    if (delay < 15) delay = 15;

    for (uint8_t pass = 0; pass < 2; pass++) {
        for (uint8_t i = 0; i < RGB_LIST_NUM; i++) {
            all_leds_off();
            nrf_gpio_pin_set(led_pins[i]);
            bsp_delay_ms(delay);
        }
        for (int8_t i = (int8_t)RGB_LIST_NUM - 2; i > 0; i--) {
            all_leds_off();
            nrf_gpio_pin_set(led_pins[i]);
            bsp_delay_ms(delay);
        }
    }
    all_leds_off();
}

// Sparkle: random quick flashes
static void effect_sparkle(uint8_t color, uint8_t speed) {
    uint32_t *led_pins = hw_get_led_array();
    set_slot_light_color((chameleon_rgb_type_t)color);
    uint8_t flashes = speed * 3 + 10;

    for (uint8_t f = 0; f < flashes; f++) {
        all_leds_off();
        uint8_t r1 = rand() % RGB_LIST_NUM;
        uint8_t r2 = rand() % RGB_LIST_NUM;
        nrf_gpio_pin_set(led_pins[r1]);
        nrf_gpio_pin_set(led_pins[r2]);
        bsp_delay_ms(25);
    }
    all_leds_off();
}

// Symmetric Burst: from center outward
static void effect_symmetric_burst(uint8_t color, uint8_t speed) {
    uint32_t *led_pins = hw_get_led_array();
    set_slot_light_color((chameleon_rgb_type_t)color);
    uint16_t delay = 80 - (speed * 6);
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

// Solid lit for 300ms
static void effect_solid(uint8_t color) {
    set_slot_light_color((chameleon_rgb_type_t)color);
    all_leds_on();
    bsp_delay_ms(300);
    all_leds_off();
}

static void run_effect(uint8_t effect, uint8_t color, uint8_t speed, uint8_t brightness) {
    switch (effect) {
        case ANIM_EFFECT_PULSE_WHITE:
            effect_pulse(RGB_WHITE, speed, brightness);
            break;
        case ANIM_EFFECT_PULSE_COLOR:
            effect_pulse(color, speed, brightness);
            break;
        case ANIM_EFFECT_SWEEP_WAVE:
            effect_sweep_wave(color, speed);
            break;
        case ANIM_EFFECT_BREATHE:
            effect_pulse(color, speed, brightness);
            break;
        case ANIM_EFFECT_RAINBOW:
            effect_rainbow(speed);
            break;
        case ANIM_EFFECT_SLOT_CHASE:
            effect_slot_chase(color, speed);
            break;
        case ANIM_EFFECT_SPARKLE:
            effect_sparkle(color, speed);
            break;
        case ANIM_EFFECT_SOLID:
            effect_solid(color);
            break;
        case ANIM_EFFECT_SYMMETRIC_BURST:
            effect_symmetric_burst(color, speed);
            break;
        case ANIM_EFFECT_NONE:
        default:
            all_leds_off();
            break;
    }
}

void custom_anim_play_connect(void) {
    run_effect(m_config.on_connect_effect,
               m_config.on_connect_color,
               m_config.on_connect_speed,
               m_config.brightness);
}

void custom_anim_play_disconnect(void) {
    run_effect(m_config.on_disconnect_effect,
               m_config.on_disconnect_color,
               m_config.on_disconnect_speed,
               m_config.brightness);
}

void custom_anim_play_test(uint8_t effect, uint8_t color, uint8_t speed) {
    run_effect(effect, color, speed, m_config.brightness);
}

void custom_anim_stop(void) {
    all_leds_off();
    rgb_marquee_reset();
}

// Non-blocking step for idle state
void custom_anim_idle_step(void) {
    if (m_config.idle_effect == ANIM_EFFECT_NONE) {
        return;
    }

    if (m_idle_timer == NULL) {
        m_idle_timer = bsp_obtain_timer(0);
    }

    uint16_t delay = 40 - (m_config.idle_speed * 3);
    if (delay < 10) delay = 10;

    if (!NO_TIMEOUT_1MS(m_idle_timer, delay)) {
        bsp_set_timer(m_idle_timer, 0);

        uint32_t *led_pins = hw_get_led_array();
        set_slot_light_color((chameleon_rgb_type_t)m_config.idle_color);

        if (m_config.idle_effect == ANIM_EFFECT_BREATHE) {
            m_breathe_val += (m_breathe_dir * 3);
            if (m_breathe_val >= 99) {
                m_breathe_val = 99;
                m_breathe_dir = -1;
            } else if (m_breathe_val <= 0) {
                m_breathe_val = 0;
                m_breathe_dir = 1;
            }
            if (m_breathe_val > 50) {
                all_leds_on();
            } else {
                all_leds_off();
            }
        } else if (m_config.idle_effect == ANIM_EFFECT_SLOT_CHASE) {
            all_leds_off();
            nrf_gpio_pin_set(led_pins[m_idle_step % RGB_LIST_NUM]);
            m_idle_step++;
        }
    }
}
