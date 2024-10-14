/* Copyright 2022 Micah Wine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H
#include "joystick.h"
//#include "i2c_master.h"
#include "analog.h"
#include "analog_sticks.h"
#include "triggers.h"
#include "special_buttons.h"

#define TIMEOUT 50

// TODO: remove patch
#ifdef PROTOCOL_CHIBIOS
#    pragma message("ChibiOS is currently 'best effort' and might not report accurate results")

i2c_status_t i2c_start_bodge(uint8_t address, uint16_t timeout) {
    i2c_start(address);

    // except on ChibiOS where the only way is do do "something"
    uint8_t data = 0;
    return i2c_readReg(address, 0, &data, sizeof(data), TIMEOUT);
}

#    define i2c_start i2c_start_bodge
#endif

// Defines names for use in layer keycodes and the keymap
enum layer_names { _JOYSTICK_RL, _JOYSTICK_RL_BETA, _JOYSTICK_VANILLA, _JOYSTICK_FIGHTSTICK, _WASD_GAMING };

// Defines the ke
// ycodes used by our macros in process_record_user
enum custom_keycodes { WD = SAFE_RANGE, WD_MANUAL, ANALOG_AIR, ANALOG_AIR_LEFT, ANALOG_AIR_RIGHT, AIR_LEFT, AIR_RIGHT, AIR_ROLL, JOYSTICK_LT_TOGGLE, CALIBRATE_JOYSTICKS, XBOX_LT, XBOX_RT };

// enum controller_mappings { XBOX_A = 3, XBOX_B = 19, XBOX_X = 7, XBOX_Y = 9, XBOX_LB = 2, XBOX_RB = 4, XBOX_BACK = 10, XBOX_START = 14, XBOX_LS = 1, XBOX_RS = 8, XBOX_UP = 17, XBOX_DOWN = 18, XBOX_LEFT = 11, XBOX_RIGHT = 12, XBOX_LT_TOG = JOYSTICK_LT_TOGGLE };
// enum controller_mappings { XBOX_A = JS_BUTTON3, XBOX_B = JS_BUTTON19, XBOX_X = JS_BUTTON7, XBOX_Y = JS_BUTTON9, XBOX_LB = JS_BUTTON2, XBOX_RB = JS_BUTTON4, XBOX_BACK = JS_BUTTON10, XBOX_START = JS_BUTTON14, XBOX_LS = JS_BUTTON1, XBOX_RS = JS_BUTTON8, XBOX_UP = JS_BUTTON17, XBOX_DOWN = JS_BUTTON18, XBOX_LEFT = JS_BUTTON11, XBOX_RIGHT = JS_BUTTON12, XBOX_LT_TOG = JOYSTICK_LT_TOGGLE };
// macro to put button in range 0-31 -- done by subtracting JS_BUTTON0

// const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
//     [_JOYSTICK_RL] = LAYOUT(
//         KC_1,        KC_2,        KC_3,       KC_4,   KC_5,                 JS_BUTTON22, XBOX_DOWN, TO(_JOYSTICK_VANILLA),
//         XBOX_LT_TOG, XBOX_LS,     XBOX_LB,    XBOX_A, XBOX_RB,              XBOX_X,      XBOX_RS,     XBOX_Y,
//         XBOX_BACK,   XBOX_LEFT,   XBOX_RIGHT, WD,     XBOX_START,          XBOX_UP,     XBOX_DOWN,   XBOX_B,
//                      JS_BUTTON24, JS_BUTTON25,                                           JS_BUTTON26, JS_BUTTON27
//     ),
//     [_JOYSTICK_VANILLA] = LAYOUT(
//         KC_1,        XBOX_LS,     XBOX_LB,   KC_4,   KC_5,                 XBOX_RB,     XBOX_RS,   TO(_WASD_GAMING),
//         XBOX_LT_TOG, XBOX_X,      XBOX_B,    XBOX_A, XBOX_RB,              XBOX_LB,     XBOX_RB,   XBOX_Y,
//         XBOX_BACK,   XBOX_LEFT,   XBOX_RIGHT, WD,     XBOX_START,          XBOX_UP,     XBOX_DOWN, XBOX_B,
//                      JS_BUTTON24, JS_BUTTON25,                             JS_BUTTON26, RESET
//     ),
//     [_WASD_GAMING] = LAYOUT(
//         KC_TAB,        KC_Q,     KC_W,   KC_E,   KC_R,                 KC_U,     KC_I,   TO(_JOYSTICK_RL),
//         KC_LSHIFT, KC_A,      KC_S,    KC_D, KC_F,              KC_J,     KC_K,   KC_L,
//         KC_LCTRL,   KC_Z,   KC_X, KC_C,     KC_V,          KC_M,     KC_N, KC_I,
//                      KC_T, KC_G,                             KC_O, KC_P
//     )
// };

struct analog_config default_left_analog_config = {
    .h_enabled = true,
    .v_enabled = true,
    .h_axis = 0,
    .v_axis = 1,
    .h_pos_button = -1,
    .h_neg_button = -1,
    .v_pos_button = -1,
    .v_neg_button = -1,
};
struct analog_config default_right_analog_config = {
    .h_enabled = true,
    .v_enabled = true,
    .h_axis = 2,
    .v_axis = 3,
    .h_pos_button = -1,
    .h_neg_button = -1,
    .v_pos_button = -1,
    .v_neg_button = -1,
};
struct analog_config rl_left_analog_config = {
    .h_enabled = true,
    .v_enabled = true,
    .h_axis = 0,
    .v_axis = 1,
    .h_pos_button = -1,
    .h_neg_button = -1,
    .v_pos_button = -1,
    .v_neg_button = -1,
    .button_cutoff = 40,
};
struct analog_config rl_right_analog_config = {
    .h_enabled = true,
    .v_enabled = true,
    .h_axis = 2,
    .v_axis = 3,
    .h_pos_button = -1,
    .h_neg_button = -1,
    .v_pos_button = BUTTON(XBOX_DOWN),
    .v_neg_button = BUTTON(XBOX_UP),
    .button_cutoff = 40,
};
struct analog_config fightstick_left_analog_config = {
    .h_enabled = true,
    .v_enabled = true,
    .h_axis = 0,
    .v_axis = 1,
    .h_pos_button = -1,
    .h_neg_button = -1,
    .v_pos_button = -1,
    .v_neg_button = -1,
};
struct analog_config fightstick_right_analog_config = {
    .h_enabled = true,
    .v_enabled = true,
    .split_h_axis = true,
    .h_axis = 4,
    .h_axis_split = 5,
    .v_axis = 3,
    .h_pos_button = -1,
    .h_neg_button = -1,
    .v_pos_button = -1,
    .v_neg_button = -1,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_JOYSTICK_RL] = LAYOUT(
        KC_1,      KC_2,              XBOX_RB, XBOX_RB,     WD_MANUAL,  KC_3,    KC_4,             TO(_JOYSTICK_VANILLA),
        XBOX_B,   ANALOG_AIR_LEFT,   XBOX_A,                             XBOX_X,  ANALOG_AIR_RIGHT,          XBOX_Y,
        XBOX_LB, XBOX_LS,            WD_MANUAL,                          WD_MANUAL,  XBOX_RS, XBOX_Y,
        XBOX_BACK, XBOX_START,                                            XBOX_LB,   CALIBRATE_JOYSTICKS),
    // [_JOYSTICK_RL_BETA] = LAYOUT(
    //     KC_1,      KC_2,       XBOX_RB, WD_MANUAL,     WD_MANUAL,  KC_3,    KC_4,      TO(_JOYSTICK_VANILLA),
    //     XBOX_LB,   AIR_LEFT,   XBOX_A,                       XBOX_X,  AIR_RIGHT, XBOX_Y,
    //     XBOX_LEFT, XBOX_RIGHT, WD_MANUAL,                    XBOX_X, PS_AIR,    XBOX_Y,
    //     XBOX_BACK, XBOX_START,                                        XBOX_LB,   XBOX_RB),
    [_JOYSTICK_VANILLA] = LAYOUT(
        KC_1, KC_2, XBOX_RB, XBOX_A,   XBOX_B, KC_3, KC_4, TO(_JOYSTICK_FIGHTSTICK),
        XBOX_LB, XBOX_X, XBOX_A,       XBOX_B, XBOX_Y, XBOX_RB,
        XBOX_LEFT, XBOX_RIGHT, XBOX_LS,  XBOX_RS, XBOX_UP, XBOX_DOWN,
        CALIBRATE_JOYSTICKS, XBOX_BACK,           XBOX_START, RESET),
    [_JOYSTICK_FIGHTSTICK] = LAYOUT(
        KC_1, KC_2, XBOX_RS, TO(_JOYSTICK_RL),   XBOX_B, XBOX_LS, XBOX_LT, XBOX_RT,
        XBOX_LEFT, XBOX_UP, XBOX_RIGHT,       XBOX_X, XBOX_Y, XBOX_RB,
        XBOX_LEFT, XBOX_DOWN, XBOX_RIGHT,  XBOX_A, XBOX_B, XBOX_LB,
        XBOX_BACK, XBOX_START,           XBOX_LS, TO(_JOYSTICK_RL)),
    // [_WASD_GAMING] = LAYOUT(KC_TAB, KC_Q, KC_W, KC_W, KC_U, KC_U, KC_I, TO(_JOYSTICK_RL), KC_LSHIFT, KC_A, KC_S, KC_J, KC_K, KC_L, KC_LCTRL, KC_Z, KC_X, KC_M, KC_N, KC_I, KC_T, KC_G, CALIBRATE_JOYSTICKS, RESET)
    };

// joystick_config_t joystick_axes[JOYSTICK_AXES_COUNT] = {[0] = JOYSTICK_AXIS_VIRTUAL, [1] = JOYSTICK_AXIS_VIRTUAL, [2] = JOYSTICK_AXIS_VIRTUAL, [3] = JOYSTICK_AXIS_VIRTUAL, [4] = JOYSTICK_AXIS_VIRTUAL, [5] = JOYSTICK_AXIS_VIRTUAL};

// static uint8_t axesFlags = 0;
// enum axes { Precision = 1, Axis1High = 2, Axis1Low = 4, Axis2High = 8, Axis2Low = 16 };

bool USE_TAP_AXIS_1 = false;
bool USE_TAP_AXIS_2 = false;

bool process_joystick_analogread() {
    calibrateJoysticks(false, false);
    calibrateTriggers(false, false);
    if (IS_LAYER_ON(_JOYSTICK_RL)) {
        scanJoysticks(rl_left_analog_config, rl_right_analog_config);
    } else if (IS_LAYER_ON(_JOYSTICK_FIGHTSTICK)) {
        scanJoysticks(fightstick_left_analog_config, fightstick_right_analog_config);
    } else {
        scanJoysticks(default_left_analog_config, default_right_analog_config);
    }
    if (IS_LAYER_ON(_JOYSTICK_RL)) {
        scanTriggers(IS_LAYER_ON(_WASD_GAMING), 4, 5, 3, USE_TAP_AXIS_1, USE_TAP_AXIS_2, false, false);
    } else if (IS_LAYER_ON(_JOYSTICK_VANILLA)) {
        scanTriggers(IS_LAYER_ON(_WASD_GAMING), 4, 5, -1, false, false, false, false);
    }
    return true;
}

void keyboard_post_init_user(void) {
    #ifdef CONSOLE_ENABLE
      debug_enable=true;
      debug_matrix=true;
      debug_keyboard=true;
    #endif
    #ifdef CONSOLE_ENABLE
        scan_timer= timer_read();
    #endif
    #ifdef USE_I2C
    if (is_keyboard_master()) {
        i2c_init();
        scan_timer= timer_read();
    }
    #endif
}

void joystick_task(void) {
    // handle_wd();
    handle_wd_manual();
    //handle_special_powerslide();
    handle_powerslide();
    process_joystick_analogread();

    if (joystick_status.status & JS_UPDATED) {
        joystick_flush();
        // send_joystick_packet(&joystick_status);
        // joystick_status.status &= ~JS_UPDATED;
    }
}

#ifdef OLED_ENABLE
bool oled_task_user(void) {
    // check if we're calibrating analog sticks
    if (DO_CALIBRATE_JOYSTICKS) {
        // if we are, show the calibration screen
        oled_write_P(PSTR("Calibrating Joysticks"), false);
    }
    else {
        // Host Keyboard Layer Status
        oled_write_P(PSTR("Layer: "), false);

        switch (get_highest_layer(layer_state)) {
            case _JOYSTICK_RL:
                oled_write_P(PSTR("RL\n"), false);
                break;
            case _JOYSTICK_VANILLA:
                oled_write_P(PSTR("VAN\n"), false);
                break;
            case _WASD_GAMING:
                oled_write_P(PSTR("WASD\n"), false);
                break;
            default:
                // Or use the write_ln shortcut over adding '\n' to the end of your string
                oled_write_ln_P(PSTR("Undefined"), false);
        }
    }

    return false;
}
#endif

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
#ifdef CONSOLE_ENABLE
    uprintf("KL: kc: %u, col: %u, row: %u, pressed: %u\n", keycode, record->event.key.col, record->event.key.row, record->event.pressed);
#endif
    switch (keycode) {
        case WD:
            if (record->event.pressed) {
                wd_first = true;
                // SEND_STRING(SS_TAP(X_V) SS_DELAY(70) SS_TAP(X_V));
            }
            break;
        case WD_MANUAL:
            if (record->event.pressed) {
                wd_manual_first = true;
            }
            else {
                wd_manual_second = true;
            }
            break;
        case ANALOG_AIR:
            if (record->event.pressed) {
                // special_powerslide_pressed = true;
                if (IS_LAYER_ON(_JOYSTICK_RL)) {
                    USE_TAP_AXIS_1 = true;
                    USE_TAP_AXIS_2 = true;
                }
                else {
                    USE_TAP_AXIS_1 = false;
                    USE_TAP_AXIS_2 = false;
                }
            } else {
                // special_powerslide_pressed = false;
                // special_powerslide_released = true;
                USE_TAP_AXIS_1 = false;
                USE_TAP_AXIS_2 = false;
            }
            break;
        case ANALOG_AIR_LEFT:
            if (record->event.pressed) {
                USE_TAP_AXIS_1 = true;
                powerslide_state.button1_pressed = true;
            } else {
                USE_TAP_AXIS_1 = false;
                powerslide_state.button1_pressed = false;
                powerslide_state.button1_released = true;
            }
            break;
        case ANALOG_AIR_RIGHT:
            if (record->event.pressed) {
                USE_TAP_AXIS_2 = true;
                powerslide_state.button2_pressed = true;
            } else {
                USE_TAP_AXIS_2 = false;
                powerslide_state.button2_pressed = false;
                powerslide_state.button2_released = true;
            }
            break;
        case AIR_LEFT:
            if (record->event.pressed) {
                air_left_pressed = true;
            } else {
                air_left_pressed = false;
            }
            handle_air_roll_state();
            break;
        case AIR_RIGHT:
            if (record->event.pressed) {
                air_right_pressed = true;
            } else {
                air_right_pressed = false;
            }
            handle_air_roll_state();
            break;
        case AIR_ROLL:
            if (record->event.pressed) {
                air_roll_pressed = true;
            } else {
                air_roll_pressed = false;
            }
            // handle_air_roll_state();
            break;
        // case JOYSTICK_LT_TOGGLE:
        //     if (record->event.pressed) {
        //         trigger_toggle = true;
        //     } else {
        //         trigger_toggle = false;
        //     }
        //     return false;
        case CALIBRATE_JOYSTICKS:
            if (record->event.pressed) {
               DO_CALIBRATE_ANALOG = true;
               calibrateTimer = timer_read();
            }
            return false;
        case XBOX_LT:
            if (record->event.pressed) {
                fightstick_right_analog_config.h_enabled = false;
                joystick_status.axes[4] = 127;
                joystick_status.status |= JS_UPDATED;
            } else {
                fightstick_right_analog_config.h_enabled = true;
                joystick_status.axes[4] = -128;
                joystick_status.status |= JS_UPDATED;
            }
            return false;
        case XBOX_RT:
            if (record->event.pressed) {
                fightstick_right_analog_config.h_enabled = false;
                joystick_status.axes[5] = 127;
                joystick_status.status |= JS_UPDATED;
            } else {
                fightstick_right_analog_config.h_enabled = true;
                joystick_status.axes[5] = -128;
                joystick_status.status |= JS_UPDATED;
            }
            return false;
    }
    return true;
}
