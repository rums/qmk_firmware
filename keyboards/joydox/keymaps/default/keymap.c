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
enum layer_names {
    _JOYSTICK_RL,
    _JOYSTICK_VANILLA
};

// Defines the ke
// ycodes used by our macros in process_record_user
enum custom_keycodes {
    JOYSTICK_RL = SAFE_RANGE,
    JOYSTICK_VANILLA,
    WD
};

enum controller_mappings {
    XBOX_A = JS_BUTTON3,
    XBOX_B = JS_BUTTON19,
    XBOX_X = JS_BUTTON7,
    XBOX_Y = JS_BUTTON9,
    XBOX_LB = JS_BUTTON2,
    XBOX_RB = JS_BUTTON4,
    XBOX_BACK = JS_BUTTON10,
    XBOX_START = JS_BUTTON14,
    XBOX_LS = JS_BUTTON1,
    XBOX_RS = JS_BUTTON8,
    XBOX_UP = JS_BUTTON17,
    XBOX_DOWN = JS_BUTTON18,
    XBOX_LEFT = JS_BUTTON11,
    XBOX_RIGHT = JS_BUTTON12,
    XBOX_LT_TOG = JS_BUTTON31
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_JOYSTICK_RL] = LAYOUT(
        KC_1,        KC_2,        KC_3,       KC_4,   KC_5,                 JS_BUTTON22, JS_BUTTON23, TO(_JOYSTICK_VANILLA),
        XBOX_LT_TOG, XBOX_LS,     XBOX_LB,    XBOX_A, XBOX_RB,              XBOX_X,      XBOX_RS,     XBOX_Y,
        XBOX_BACK,   XBOX_LEFT,   XBOX_RIGHT, WD,     XBOX_START,          XBOX_UP,     XBOX_DOWN,   XBOX_B,
                     JS_BUTTON24, JS_BUTTON25,                                           JS_BUTTON26, JS_BUTTON27
    ),
    [_JOYSTICK_VANILLA] = LAYOUT(
        KC_1,        XBOX_LS,     XBOX_LB,   KC_4,   KC_5,                 XBOX_RB,     XBOX_RS,   TO(_JOYSTICK_RL),
        XBOX_LT_TOG, XBOX_X,      XBOX_B,    XBOX_A, XBOX_RB,              XBOX_LB,     XBOX_RB,   XBOX_Y,
        XBOX_BACK,   XBOX_LEFT,   XBOX_RIGHT, WD,     XBOX_START,          XBOX_UP,     XBOX_DOWN, XBOX_B,
                     JS_BUTTON24, JS_BUTTON25,                             JS_BUTTON26, RESET
    )
};

joystick_config_t joystick_axes[JOYSTICK_AXES_COUNT] = {
    [0] = JOYSTICK_AXIS_VIRTUAL,
    [1] = JOYSTICK_AXIS_VIRTUAL,
    [2] = JOYSTICK_AXIS_VIRTUAL,
    [3] = JOYSTICK_AXIS_VIRTUAL,
    [4] = JOYSTICK_AXIS_VIRTUAL,
    [5] = JOYSTICK_AXIS_VIRTUAL
};

// static uint8_t axesFlags = 0;
enum axes {
    Precision = 1,
    Axis1High = 2,
    Axis1Low = 4,
    Axis2High = 8,
    Axis2Low = 16
};

#define QWIIC_JOYSTICK_LEFT_ADDR (0x20 << 1)
#define QWIIC_JOYSTICK_RIGHT_ADDR (0x24 << 1)
#define QWIIC_TRIGGER_LEFT_ADDR (0x64 << 1)

uint8_t scanJoystick(int8_t joystickAddr, uint8_t axis1, uint8_t axis2) {
    uint8_t nDevices = 0;
    i2c_status_t error = i2c_start(joystickAddr, TIMEOUT);
    if (error == I2C_STATUS_SUCCESS) {
        i2c_stop();
        // registers 3 and 4 are current horizontal position (MSB first)
        // registers 5 and 6 are current vertical position (MSB first)
        // registers 7 and 8 are current button state (7 is current position, 8 is if button was pressed since last read of button state)
        uint8_t horizontalMsbData = 0;
        uint8_t horizontalLsbData = 0;
        uint8_t verticalMsbData = 0;
        uint8_t verticalLsbData = 0;
        uint8_t buttonCurrentData = 0;
        uint8_t buttonFreshData = 0;
        i2c_status_t horizontalMsb = i2c_readReg(joystickAddr, 3, &horizontalMsbData, sizeof(horizontalMsbData), TIMEOUT);
        i2c_status_t horizontalLsb = i2c_readReg(joystickAddr, 4, &horizontalLsbData, sizeof(horizontalLsbData), TIMEOUT);
        i2c_status_t verticalMsb = i2c_readReg(joystickAddr, 5, &verticalMsbData, sizeof(verticalMsbData), TIMEOUT);
        i2c_status_t verticalLsb = i2c_readReg(joystickAddr, 6, &verticalLsbData, sizeof(verticalLsbData), TIMEOUT);
        i2c_status_t buttonCurrent = i2c_readReg(joystickAddr, 7, &buttonCurrentData, sizeof(buttonCurrentData), TIMEOUT);
        i2c_status_t buttonFresh = i2c_readReg(joystickAddr, 8, &buttonFreshData, sizeof(buttonFreshData), TIMEOUT);
        if (horizontalMsb == I2C_STATUS_SUCCESS && horizontalLsb == I2C_STATUS_SUCCESS && verticalMsb == I2C_STATUS_SUCCESS && verticalLsb == I2C_STATUS_SUCCESS && buttonCurrent == I2C_STATUS_SUCCESS && buttonFresh == I2C_STATUS_SUCCESS) {
            int8_t horizontal = horizontalMsbData - 128;
            int8_t vertical = verticalMsbData - 128;
            if (joystick_status.axes[axis1] != horizontal) {
                joystick_status.axes[axis1] = horizontal;
                joystick_status.status |= JS_UPDATED;
            }
            if (joystick_status.axes[axis2] != vertical) {
                joystick_status.axes[axis2] = vertical;
                joystick_status.status |= JS_UPDATED;
            }
        } else {
            dprintf("  Error: %d %d %d %d %d %d\n", horizontalMsb, horizontalLsb, verticalMsb, verticalLsb, buttonCurrent, buttonFresh);
        }
        nDevices++;
    } else {
        dprintf("  Unknown error (%u) at address 0x%02X\n", error, joystickAddr);
    }
    return nDevices;
}

#ifdef USE_SLIDER
#define QWIIC_SLIDER_ADDR (0x28 << 1)
void scanSlider(void) {
    i2c_status_t error2 = i2c_start(QWIIC_SLIDER_ADDR, TIMEOUT);
    if (error2 == I2C_STATUS_SUCCESS) {
        i2c_stop();
        dprintf("  I2C device found at address 0x%02X\n", QWIIC_SLIDER_ADDR);
        // registers 3 and 4 are current horizontal position (MSB first)
        // registers 5 and 6 are current vertical position (MSB first)
        // registers 7 and 8 are current button state (7 is current position, 8 is if button was pressed since last read of button state)
        uint8_t sliderData = 0;
        i2c_status_t slider = i2c_readReg(QWIIC_SLIDER_ADDR, 3, &sliderData, sizeof(sliderData), TIMEOUT);
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
#endif // USE_SLIDER

void scanJoysticks(void) {
    uint8_t nDevices = 0;

    nDevices += scanJoystick(QWIIC_JOYSTICK_LEFT_ADDR, 0, 1);
    nDevices += scanJoystick(QWIIC_JOYSTICK_RIGHT_ADDR, 2, 3);
    #ifdef USE_SLIDER
    nDevices += scanSlider();
    #endif

    if (nDevices == 0)
        dprintf("No I2C devices found\n");
}

uint16_t scan_timer = 0;

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define RIGHT_TRIGGER_ZERO 516
void scanTriggers(void) {
    bool jsUpdated = false;
    int32_t rawVal = analogReadPin(F4);
    if (rawVal > RIGHT_TRIGGER_ZERO + 1) {
        uint16_t minInput = 528;
        uint16_t maxInput = 1020;
        uint8_t minOutput = 0;
        uint8_t maxOutput = 127;
        if (layer_state == _JOYSTICK_VANILLA) {
            maxInput = 850;
            if (rawVal > maxInput) {
                rawVal = maxInput;
            }
        }
        // map rawVal from input range to output range
        double slope = 1.0 * (maxOutput - minOutput) / (maxInput - minInput);
        int16_t rangedVal = MIN(ceil( slope * (rawVal - minInput) + minOutput), maxOutput);
        if (!(joystick_status.buttons[(XBOX_LT_TOG - JS_BUTTON0) / 8] & (1 << (XBOX_LT_TOG % 8)))) {
            if (joystick_status.axes[5] != 0 || joystick_status.axes[4] != rangedVal) {
                joystick_status.axes[5] = 0;
                joystick_status.axes[4] = rangedVal;
                jsUpdated = true;
            }
        }
        else {
            if (joystick_status.axes[4] != 0 || joystick_status.axes[5] != rangedVal) {
                joystick_status.axes[4] = 0;
                joystick_status.axes[5] = rangedVal;
                jsUpdated = true;
            }
        }
        }
    else {
        // set both axes to 0
        int16_t rangedVal = 0;
        if (rangedVal != joystick_status.axes[4]) {
            joystick_status.axes[4] = rangedVal;
            jsUpdated = true;
        }
        if (rangedVal != joystick_status.axes[5]) {
            joystick_status.axes[5] = rangedVal;
            jsUpdated = true;
        }
    }
    if (jsUpdated) {
        joystick_status.status |= JS_UPDATED;
    }
    #ifdef CONSOLE_ENABLE
    if (timer_elapsed(scan_timer) > 200) {
        uprintf("rawVal: %d\n", rawVal);
        uprintf("axis 4: %d\n", joystick_status.axes[4]);
        uprintf("axis 5: %d\n", joystick_status.axes[5]);
        scan_timer = timer_read();
    }
    #endif
}

bool process_joystick_analogread() {
    scanJoysticks();
    scanTriggers();
    return true;
}

void keyboard_post_init_user(void) {
    // debug_enable = true;
    // debug_matrix = true;

    i2c_init();
    scan_timer = timer_read();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    #ifdef CONSOLE_ENABLE
        uprintf("KL: kc: %u, col: %u, row: %u, pressed: %u\n", keycode, record->event.key.col, record->event.key.row, record->event.pressed);
    #endif
    switch (keycode) {
        case WD:
            if (record->event.pressed) {
                SEND_STRING(SS_TAP(X_V) SS_DELAY(70) SS_TAP(X_V));
            }
    }
    return true;
}
