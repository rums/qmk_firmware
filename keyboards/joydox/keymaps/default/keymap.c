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
    _BASE,
    _FN
};

// Defines the ke
// ycodes used by our macros in process_record_user
enum custom_keycodes {
    QMKBEST = SAFE_RANGE,
    QMKURL
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(
        KC_A, KC_B, KC_C, KC_D, KC_E,   KC_F, KC_G, KC_H, KC_I, KC_J,
        KC_K, KC_L, KC_M, KC_N, KC_O,   KC_P, KC_Q, KC_R, KC_S, KC_T,
        KC_U, KC_V, KC_W, KC_X, KC_Y,   KC_Z, KC_1, KC_2, KC_3, KC_4,
                    KC_5, KC_6,               KC_7, KC_8
    )
};

joystick_config_t joystick_axes[JOYSTICK_AXES_COUNT] = {
    [0] = JOYSTICK_AXIS_VIRTUAL,
    [1] = JOYSTICK_AXIS_VIRTUAL
};

// static uint8_t axesFlags = 0;
enum axes {
    Precision = 1,
    Axis1High = 2,
    Axis1Low = 4,
    Axis2High = 8,
    Axis2Low = 16
};

#define QWIIC_JOYSTICK_ADDR (0x20 << 1)
void do_scan(void) {
    uint8_t nDevices = 0;

    // dprintf("Scanning...\n");

    i2c_status_t error = i2c_start(QWIIC_JOYSTICK_ADDR, TIMEOUT);
    if (error == I2C_STATUS_SUCCESS) {
        i2c_stop();
        // dprintf("  I2C device found at address 0x%02X\n", QWIIC_JOYSTICK_ADDR);
        // registers 3 and 4 are current horizontal position (MSB first)
        // registers 5 and 6 are current vertical position (MSB first)
        // registers 7 and 8 are current button state (7 is current position, 8 is if button was pressed since last read of button state)
        uint8_t horizontalMsbData = 0;
        uint8_t horizontalLsbData = 0;
        uint8_t verticalMsbData = 0;
        uint8_t verticalLsbData = 0;
        uint8_t buttonCurrentData = 0;
        uint8_t buttonFreshData = 0;
        i2c_status_t horizontalMsb = i2c_readReg(QWIIC_JOYSTICK_ADDR, 3, &horizontalMsbData, sizeof(horizontalMsbData), TIMEOUT);
        i2c_status_t horizontalLsb = i2c_readReg(QWIIC_JOYSTICK_ADDR, 4, &horizontalLsbData, sizeof(horizontalLsbData), TIMEOUT);
        i2c_status_t verticalMsb = i2c_readReg(QWIIC_JOYSTICK_ADDR, 5, &verticalMsbData, sizeof(verticalMsbData), TIMEOUT);
        i2c_status_t verticalLsb = i2c_readReg(QWIIC_JOYSTICK_ADDR, 6, &verticalLsbData, sizeof(verticalLsbData), TIMEOUT);
        i2c_status_t buttonCurrent = i2c_readReg(QWIIC_JOYSTICK_ADDR, 7, &buttonCurrentData, sizeof(buttonCurrentData), TIMEOUT);
        i2c_status_t buttonFresh = i2c_readReg(QWIIC_JOYSTICK_ADDR, 8, &buttonFreshData, sizeof(buttonFreshData), TIMEOUT);
        if (horizontalMsb == I2C_STATUS_SUCCESS && horizontalLsb == I2C_STATUS_SUCCESS && verticalMsb == I2C_STATUS_SUCCESS && verticalLsb == I2C_STATUS_SUCCESS && buttonCurrent == I2C_STATUS_SUCCESS && buttonFresh == I2C_STATUS_SUCCESS) {
            int8_t horizontal = horizontalMsbData - 128;
            int8_t vertical = verticalMsbData - 128;
            joystick_status.axes[0] = horizontal;
            joystick_status.axes[1] = vertical;
            joystick_status.status |= JS_UPDATED;
        } else {
            dprintf("  Error: %d %d %d %d %d %d\n", horizontalMsb, horizontalLsb, verticalMsb, verticalLsb, buttonCurrent, buttonFresh);
        }
        nDevices++;
    } else {
        dprintf("  Unknown error (%u) at address 0x%02X\n", error, QWIIC_JOYSTICK_ADDR);
    }

    if (nDevices == 0)
        dprintf("No I2C devices found\n");
}

uint16_t scan_timer = 0;

// void matrix_scan_user(void) {
    // /* joystick stuff */
    // int16_t val = (((uint32_t)timer_read() % 5000 - 2500) * 255) / 5000;

    // if (val != joystick_status.axes[0]) {
    //     joystick_status.axes[0] = val;
    //     joystick_status.status |= JS_UPDATED;
    // }
// }

// void joystick_task(void) {
//     /* analog stuff */
//     if (timer_elapsed(scan_timer) > 100) {
//         do_scan();
//         scan_timer = timer_read();
//     }
//     if (joystick_status.status & JS_UPDATED) {
//         send_joystick_packet(&joystick_status);
//         joystick_status.status &= ~JS_UPDATED;
//     }
// }

bool process_joystick_analogread() {
    return false;
    if (timer_elapsed(scan_timer) > 100) {
        do_scan();
        scan_timer = timer_read();
    }
    return true;
}

void keyboard_post_init_user(void) {
    debug_enable = true;
    debug_matrix = true;

    return;
    i2c_init();
    scan_timer = timer_read();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    #ifdef CONSOLE_ENABLE
        uprintf("KL: kc: %u, col: %u, row: %u, pressed: %u\n", keycode, record->event.key.col, record->event.key.row, record->event.pressed);
    #endif
  return true;
//     SEND_STRING("process_record_user\n");
//     switch (keycode) {
//         case QMKBEST:
//             if (record->event.pressed) {
//                 // when keycode QMKBEST is pressed
//                 SEND_STRING("QMK is the best thing ever!");
//             } else {
//                 // when keycode QMKBEST is released
//             }
//             break;
//         case QMKURL:
//             if (record->event.pressed) {
//                 // when keycode QMKURL is pressed
//                 SEND_STRING("https://qmk.fm/\n");
//             } else {
//                 // when keycode QMKURL is released
//             }
//             break;
//     }
//     return true;
}
