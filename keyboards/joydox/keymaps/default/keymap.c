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
#include "i2c_master.h"
#include "analog.h"

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
enum layer_names { _JOYSTICK_RL, _JOYSTICK_VANILLA, _WASD_GAMING };

// Defines the ke
// ycodes used by our macros in process_record_user
enum custom_keycodes { JOYSTICK_RL = SAFE_RANGE, JOYSTICK_VANILLA, WASD_GAMING, WD, PS_AIR, JOYSTICK_LT_TOGGLE };

enum controller_mappings { XBOX_A = JS_BUTTON3, XBOX_B = JS_BUTTON19, XBOX_X = JS_BUTTON7, XBOX_Y = JS_BUTTON9, XBOX_LB = JS_BUTTON2, XBOX_RB = JS_BUTTON4, XBOX_BACK = JS_BUTTON10, XBOX_START = JS_BUTTON14, XBOX_LS = JS_BUTTON1, XBOX_RS = JS_BUTTON8, XBOX_UP = JS_BUTTON17, XBOX_DOWN = JS_BUTTON18, XBOX_LEFT = JS_BUTTON11, XBOX_RIGHT = JS_BUTTON12, XBOX_LT_TOG = JOYSTICK_LT_TOGGLE };

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

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_JOYSTICK_RL] = LAYOUT(
        KC_1, KC_2, XBOX_RB,        KC_3, KC_4, TO(_JOYSTICK_VANILLA),
        PS_AIR, XBOX_LS, XBOX_A,    XBOX_X, XBOX_RS, XBOX_Y,
        XBOX_LEFT, XBOX_RIGHT, WD,  XBOX_UP, XBOX_B, XBOX_DOWN,
        XBOX_BACK, XBOX_START,      XBOX_LB, XBOX_RB),
    [_JOYSTICK_VANILLA] = LAYOUT(KC_1, KC_2, XBOX_RB, KC_3, KC_4, TO(_WASD_GAMING), XBOX_LS, XBOX_LB, XBOX_A, XBOX_B, XBOX_RB, XBOX_RS, XBOX_LEFT, XBOX_RIGHT, XBOX_X, XBOX_Y, XBOX_UP, XBOX_DOWN, XBOX_LEFT, XBOX_BACK, XBOX_START, XBOX_DOWN),
    [_WASD_GAMING] = LAYOUT(KC_TAB, KC_Q, KC_W, KC_U, KC_I, TO(_JOYSTICK_RL), KC_LSHIFT, KC_A, KC_S, KC_J, KC_K, KC_L, KC_LCTRL, KC_Z, KC_X, KC_M, KC_N, KC_I, KC_T, KC_G, KC_O, RESET)
    };

joystick_config_t joystick_axes[JOYSTICK_AXES_COUNT] = {[0] = JOYSTICK_AXIS_VIRTUAL, [1] = JOYSTICK_AXIS_VIRTUAL, [2] = JOYSTICK_AXIS_VIRTUAL, [3] = JOYSTICK_AXIS_VIRTUAL, [4] = JOYSTICK_AXIS_VIRTUAL, [5] = JOYSTICK_AXIS_VIRTUAL};

// static uint8_t axesFlags = 0;
enum axes { Precision = 1, Axis1High = 2, Axis1Low = 4, Axis2High = 8, Axis2Low = 16 };

int32_t max_x = 0;
int32_t max_y = 0;

#define QWIIC_JOYSTICK_LEFT_ADDR (0x20 << 1)
#define QWIIC_JOYSTICK_RIGHT_ADDR (0x24 << 1)
#define QWIIC_TRIGGER_LEFT_ADDR (0x64 << 1)

struct joystick_data {
    bool    success;
    int8_t  horizontal;
    int8_t  vertical;
    uint8_t buttonCurrent;
};

int8_t applyCurve(int8_t value) {
    int8_t sign = value > 0 ? 1 : -1;

    // deadzone
    if (value > -5 && value < 1) {
        return 0;
    }
    // snap to 45
    if (abs(value) <= 45) return 45 * sign;
    return value;
}

int16_t mapToRange(int16_t value, int16_t min, int16_t max, int8_t min_out, int8_t max_out) {
    double slope = 1.0 * (max_out - min_out) / (max - min);
    return MIN(ceil(slope * (value - min) + min_out), max_out);
}

uint16_t scan_timer     = 0;
bool     trigger_toggle = false;

struct joystick_data scanJoystick(int8_t joystickAddr, bool curveHorizontal) {
    i2c_status_t error = i2c_start(joystickAddr, TIMEOUT);
    if (error == I2C_STATUS_SUCCESS) {
        i2c_stop();
        // registers 3 and 4 are current horizontal position (MSB first)
        // registers 5 and 6 are current vertical position (MSB first)
        // registers 7 and 8 are current button state (7 is current position, 8 is if button was pressed since last read of button state)
        uint8_t      horizontalMsbData = 0;
        uint8_t      horizontalLsbData = 0;
        uint8_t      verticalMsbData   = 0;
        uint8_t      verticalLsbData   = 0;
        uint8_t      buttonCurrentData = 0;
        uint8_t      buttonFreshData   = 0;
        i2c_status_t horizontalMsb     = i2c_readReg(joystickAddr, 3, &horizontalMsbData, sizeof(horizontalMsbData), TIMEOUT);
        i2c_status_t horizontalLsb     = i2c_readReg(joystickAddr, 4, &horizontalLsbData, sizeof(horizontalLsbData), TIMEOUT);
        i2c_status_t verticalMsb       = i2c_readReg(joystickAddr, 5, &verticalMsbData, sizeof(verticalMsbData), TIMEOUT);
        i2c_status_t verticalLsb       = i2c_readReg(joystickAddr, 6, &verticalLsbData, sizeof(verticalLsbData), TIMEOUT);
        i2c_status_t buttonCurrent     = i2c_readReg(joystickAddr, 7, &buttonCurrentData, sizeof(buttonCurrentData), TIMEOUT);
        i2c_status_t buttonFresh       = i2c_readReg(joystickAddr, 8, &buttonFreshData, sizeof(buttonFreshData), TIMEOUT);
        if (horizontalMsb == I2C_STATUS_SUCCESS && horizontalLsb == I2C_STATUS_SUCCESS && verticalMsb == I2C_STATUS_SUCCESS && verticalLsb == I2C_STATUS_SUCCESS && buttonCurrent == I2C_STATUS_SUCCESS && buttonFresh == I2C_STATUS_SUCCESS) {
            int8_t horizontal = horizontalMsbData - 128;
            int8_t vertical   = verticalMsbData - 128;
            return (struct joystick_data){.success = true, .horizontal = curveHorizontal ? applyCurve(horizontal) : horizontal, .vertical = vertical, .buttonCurrent = buttonCurrentData};
        } else {
            dprintf("  Error: %d %d %d %d %d %d\n", horizontalMsb, horizontalLsb, verticalMsb, verticalLsb, buttonCurrent, buttonFresh);
        }
    } else {
        dprintf("  Unknown error (%u) at address 0x%02X\n", error, joystickAddr);
    }
    return (struct joystick_data){.success = false, .horizontal = 0, .vertical = 0, .buttonCurrent = 0};
}

uint8_t scanJoystick_controller(int8_t joystickAddr, uint8_t axis1, uint8_t axis2, bool horizontalCurve) {
    uint8_t              nDevices     = 0;
    struct joystick_data joystickData = scanJoystick(joystickAddr, horizontalCurve);
    if (joystickData.success) {
#ifdef CONSOLE_ENABLE
        bool axesChanged = false;
#endif
        if (joystick_status.axes[axis1] != joystickData.horizontal) {
            joystick_status.axes[axis1] = joystickData.horizontal;
            joystick_status.status |= JS_UPDATED;
#ifdef CONSOLE_ENABLE
            axesChanged = true;
#endif
        }
        if (joystick_status.axes[axis2] != joystickData.vertical) {
            joystick_status.axes[axis2] = joystickData.vertical;
            joystick_status.status |= JS_UPDATED;
#ifdef CONSOLE_ENABLE
            axesChanged = true;
#endif
        }
#ifdef CONSOLE_ENABLE
        if (axesChanged) {
            uprintf("%d h: %d \t\t %d v: %d\n", joystickAddr, joystickData.horizontal, joystickAddr, joystickData.vertical);
        }
#endif
        nDevices++;
    }
    return nDevices;
}

struct joystick_data scanJoystick_controller_analog(uint8_t horizontalPin, uint8_t verticalPin, uint8_t axis1, uint8_t axis2, uint16_t h_min, uint16_t h_max, uint16_t v_min, uint16_t v_max, uint8_t deadzone, uint16_t h_zero, uint16_t v_zero) {
    int16_t horizontal = analogReadPin(horizontalPin);
    int16_t vertical   = analogReadPin(verticalPin);

    if (horizontal < h_min) horizontal = h_min;
    if (horizontal > h_max) horizontal = h_max;
    if (vertical < v_min) vertical = v_min;
    if (vertical > v_max) vertical = v_max;
    int8_t horizontal_mapped = 0;
    int8_t vertical_mapped   = 0;
    if (horizontal >= h_zero + deadzone) {
        horizontal_mapped = mapToRange(horizontal, h_zero + deadzone, h_max, 0, 127);
    } else if (horizontal <= h_zero - deadzone) {
        horizontal_mapped = mapToRange(horizontal, h_min, h_zero - deadzone, -128, 0);
    }
    if (vertical >= v_zero + deadzone) {
        vertical_mapped = mapToRange(vertical, v_zero + deadzone, v_max, 0, 127);
    } else if (vertical <= v_zero - deadzone) {
        vertical_mapped = mapToRange(vertical, v_min, v_zero - deadzone, -128, 0);
    }

    if (joystick_status.axes[axis1] != horizontal_mapped) {
        joystick_status.axes[axis1] = horizontal_mapped;
        joystick_status.status |= JS_UPDATED;
    }
    if (joystick_status.axes[axis2] != vertical_mapped) {
        joystick_status.axes[axis2] = vertical_mapped;
        joystick_status.status |= JS_UPDATED;
    }
#ifdef CONSOLE_ENABLE
    // uprintf("h: %d\t\t", horizontal);
    // uprintf("v: %d\n", vertical);
#endif
    return (struct joystick_data){.success = true, .horizontal = horizontal_mapped, .vertical = vertical_mapped, .buttonCurrent = 0};
}

uint8_t scanJoystick_wasd(int8_t joystickAddr, bool isLeft) {
    uint8_t nDevices = 0;
    // joystick_data joystickData = scanJoystick(joystickAddr);
    // implement layer changes based on joystick position
    return nDevices;
}

#ifdef USE_SLIDER
#    define QWIIC_SLIDER_ADDR (0x28 << 1)
void scanSlider(void) {
    i2c_status_t error2 = i2c_start(QWIIC_SLIDER_ADDR, TIMEOUT);
    if (error2 == I2C_STATUS_SUCCESS) {
        i2c_stop();
        dprintf("  I2C device found at address 0x%02X\n", QWIIC_SLIDER_ADDR);
        // registers 3 and 4 are current horizontal position (MSB first)
        // registers 5 and 6 are current vertical position (MSB first)
        // registers 7 and 8 are current button state (7 is current position, 8 is if button was pressed since last read of button state)
        uint8_t      sliderData = 0;
        i2c_status_t slider     = i2c_readReg(QWIIC_SLIDER_ADDR, 3, &sliderData, sizeof(sliderData), TIMEOUT);
        if (slider == I2C_STATUS_SUCCESS) {
            joystick_status.axes[2] = sliderData - 128;
            joystick_status.status |= JS_UPDATED;
        } else {
            dprintf("  Error: %d\n", slider);
        }
        nDevices++;
    } else {
        dprintf("  Unknown error (%u) at address 0x%02X\n", error2, QWIIC_SLIDER_ADDR);
    }
}
#endif  // USE_SLIDER

void scanJoysticks(void) {
    uint8_t nDevices = 0;

    uint16_t l_h_min    = 165;
    uint16_t l_h_max    = 882;
    uint16_t l_v_min    = 214;
    uint16_t l_v_max    = 906;
    uint16_t l_deadzone = 10;
    uint16_t l_h_zero   = 505;
    uint16_t l_v_zero   = 515;

    uint16_t r_h_min    = 130;
    uint16_t r_h_max    = 750;
    uint16_t r_v_min    = 207;
    uint16_t r_v_max    = 845;
    uint16_t r_deadzone = 20;
    uint16_t r_h_zero   = 484;
    uint16_t r_v_zero   = 500;

// nDevices += scanJoystick_controller(QWIIC_JOYSTICK_LEFT_ADDR, 0, 1, false);
// nDevices += scanJoystick_controller(QWIIC_JOYSTICK_RIGHT_ADDR, 2, 3, true);
#ifdef CONSOLE_ENABLE
    struct joystick_data left  = scanJoystick_controller_analog(LEFT_ANALOG_HORIZONTAL, LEFT_ANALOG_VERTICAL, 0, 1, l_h_min, l_h_max, l_v_min, l_v_max, l_deadzone, l_h_zero, l_v_zero);
    struct joystick_data right = scanJoystick_controller_analog(RIGHT_ANALOG_HORIZONTAL, RIGHT_ANALOG_VERTICAL, 2, 3, r_h_min, r_h_max, r_v_min, r_v_max, r_deadzone, r_h_zero, r_v_zero);
#else
    scanJoystick_controller_analog(LEFT_ANALOG_HORIZONTAL, LEFT_ANALOG_VERTICAL, 0, 1, l_h_min, l_h_max, l_v_min, l_v_max, l_deadzone, l_h_zero, l_v_zero);
    scanJoystick_controller_analog(RIGHT_ANALOG_HORIZONTAL, RIGHT_ANALOG_VERTICAL, 2, 3, r_h_min, r_h_max, r_v_min, r_v_max, r_deadzone, r_h_zero, r_v_zero);
#endif
#ifdef CONSOLE_ENABLE
    // if 100ms have passed
    if (timer_elapsed(scan_timer) > 200) {
        uprintf(" left h: %d\t\t", left.horizontal);
        uprintf(" left v: %d\t\t\t\t", left.vertical);
        uprintf(" right h: %d\t\t", right.horizontal);
        uprintf(" right v: %d\n", right.vertical);
        // uprintf(" left h_mapped: %d\t\t", horizontal_mapped);
        // uprintf(" right v_mapped: %d\n", vertical_mapped);
        scan_timer = timer_read();
    }
#endif
#ifdef USE_SLIDER
    nDevices += scanSlider();
#endif

    if (nDevices == 0) dprintf("No I2C devices found\n");
}

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void controllerTriggers(uint8_t axis, int32_t rawVal, uint16_t axisZero, uint16_t minInput, uint16_t maxInput) {
    bool jsUpdated = false;
    if (rawVal > axisZero + 1) {
        if (rawVal > maxInput) {
            rawVal = maxInput;
        }
        uint8_t minOutput = 0;
        uint8_t maxOutput = 127;
        // map rawVal from input range to output range
        int16_t rangedVal = mapToRange(rawVal, minInput, maxInput, minOutput, maxOutput);
        if (joystick_status.axes[axis] != rangedVal) {
            joystick_status.axes[axis] = rangedVal;
            jsUpdated                  = true;
        }
    } else {
        int16_t rangedVal = 0;
        if (rangedVal != joystick_status.axes[axis]) {
            joystick_status.axes[axis] = rangedVal;
            jsUpdated                  = true;
        }
    }
    if (jsUpdated) {
        joystick_status.status |= JS_UPDATED;
    }
}

void wasdTriggers(int32_t rawVal, uint16_t axisZero, uint16_t minInput, uint16_t maxInput) {
    if (rawVal > axisZero + 1) {
        uint8_t minOutput = 0;
        uint8_t maxOutput = 127;
        if (rawVal > maxInput) {
            rawVal = maxInput;
        }
        // map rawVal from input range to output range
        double  slope     = 1.0 * (maxOutput - minOutput) / (maxInput - minInput);
        int16_t rangedVal = MIN(ceil(slope * (rawVal - minInput) + minOutput), maxOutput);
        if (rangedVal == maxOutput && !trigger_toggle) {
            unregister_code(KC_B);
            register_code(KC_Y);
        } else if (rangedVal == maxOutput) {
            unregister_code(KC_Y);
            register_code(KC_B);
        } else {
            unregister_code(KC_B);
            unregister_code(KC_Y);
        }
    }
}

void scanTriggers(void) {
    int32_t rightRawVal = analogReadPin(RIGHT_TRIGGER_PIN);
    int32_t leftRawVal  = analogReadPin(LEFT_TRIGGER_PIN);
    if (IS_LAYER_ON(_WASD_GAMING)) {
        //wasdTriggers(leftRawVal, 510, 288, 510);
        //wasdTriggers(rightRawVal, 516, 528, 750);
    } else {
        leftRawVal = 1023 - leftRawVal;
        controllerTriggers(4, leftRawVal, 513, 528, 980);
        controllerTriggers(5, rightRawVal, 516, 528, 980);
    }
    // #ifdef CONSOLE_ENABLE
    // if (timer_elapsed(scan_timer) > 200) {
    //     uprintf("leftRawVal: %d\n", leftRawVal);
    //     uprintf("rightRawVal: %d\n", rightRawVal);
    //     uprintf("axis 4: %d\n", joystick_status.axes[4]);
    //     uprintf("axis 5: %d\n", joystick_status.axes[5]);
    //     scan_timer = timer_read();
    // }
    // #endif
}

bool process_joystick_analogread() {
    if (is_keyboard_master()) {
        scanJoysticks();
        scanTriggers();
        return true;
    }
    return false;
}

void keyboard_post_init_user(void) {
    // debug_enable = true;
    // debug_matrix = true;
    if (is_keyboard_master()) {
        i2c_init();
        scan_timer = timer_read();
    }
}

uint16_t wd_timer;
bool     wd_first  = false;
bool     wd_second = false;
bool     wd_stop   = false;

void handle_wd(void) {
    if (wd_first) {
        joystick_status.buttons[(XBOX_A - JS_BUTTON0) / 8] |= 1 << (XBOX_A % 8);
        joystick_status.status |= JS_UPDATED;
        wd_first  = false;
        wd_second = true;
        wd_stop   = true;
        wd_timer  = timer_read();
    } else if (wd_second && (timer_elapsed(wd_timer) > 65)) {
        joystick_status.buttons[(XBOX_A - JS_BUTTON0) / 8] |= 1 << (XBOX_A % 8);
        joystick_status.status |= JS_UPDATED;
        wd_second = false;
        wd_stop   = true;
    } else if (wd_stop) {
        joystick_status.buttons[(XBOX_A - JS_BUTTON0) / 8] &= ~(1 << (XBOX_A % 8));
        joystick_status.status |= JS_UPDATED;
        wd_stop = false;
    }
}

bool special_powerslide_pressed = false;
bool special_powerslide_released = false;

void handle_special_powerslide(void) {
    if (special_powerslide_pressed) {
        // if air roll left or right are pressed, don't air roll
        if (joystick_status.buttons[(XBOX_RS - JS_BUTTON0) / 8] & (1 << (XBOX_RS % 8)) || joystick_status.buttons[(XBOX_LS - JS_BUTTON0) / 8] & (1 << (XBOX_LS % 8))) {
            joystick_status.buttons[(XBOX_LB - JS_BUTTON0) / 8] |= 1 << (XBOX_LB % 8);
            joystick_status.buttons[(XBOX_B - JS_BUTTON0) / 8] &= ~(1 << (XBOX_B % 8));
            joystick_status.status |= JS_UPDATED;
        } else {
            joystick_status.buttons[(XBOX_LB - JS_BUTTON0) / 8] |= 1 << (XBOX_LB % 8);
            joystick_status.buttons[(XBOX_B - JS_BUTTON0) / 8] |= 1 << (XBOX_B % 8);
            joystick_status.status |= JS_UPDATED;
        }
    } else if (special_powerslide_released) {
        joystick_status.buttons[(XBOX_LB - JS_BUTTON0) / 8] &= ~(1 << (XBOX_LB % 8));
        joystick_status.buttons[(XBOX_B - JS_BUTTON0) / 8] &= ~(1 << (XBOX_B % 8));
        joystick_status.status |= JS_UPDATED;
        special_powerslide_released = false;
    }
}

void joystick_task(void) {
    handle_wd();
    handle_special_powerslide();

    if (process_joystick_analogread() && (joystick_status.status & JS_UPDATED)) {
        send_joystick_packet(&joystick_status);
        joystick_status.status &= ~JS_UPDATED;
    }
}

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
        case PS_AIR:
            if (record->event.pressed) {
                special_powerslide_pressed = true;
            } else {
                special_powerslide_pressed = false;
                special_powerslide_released = true;
            }
        case JOYSTICK_LT_TOGGLE:
            if (record->event.pressed) {
                trigger_toggle = true;
            } else {
                trigger_toggle = false;
            }
            return false;
    }
    return true;
}
