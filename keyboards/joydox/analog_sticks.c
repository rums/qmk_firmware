#include QMK_KEYBOARD_H
#include "joystick.h"
#include "analog.h"
#include "analog_sticks.h"
#include "math.h"

#define QWIIC_JOYSTICK_LEFT_ADDR (0x20 << 1)
#define QWIIC_JOYSTICK_RIGHT_ADDR (0x24 << 1)
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

uint16_t scan_timer          = 0;
bool     DO_CALIBRATE_ANALOG = false;
uint16_t calibrateTimer      = 0;

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

int16_t mapToRange_analog(int16_t value, int16_t min, int16_t max, int8_t min_out, int8_t max_out) {
    double slope = 1.0 * (max_out - min_out) / (max - min);
    return (int16_t)(slope * (value - min) + min_out);
}

struct joystick_data scanJoystick_controller_analog(uint8_t horizontalPin, uint8_t verticalPin, struct analog_config config, uint16_t h_min, uint16_t h_max, uint16_t v_min, uint16_t v_max, uint8_t deadzone, uint16_t h_zero, uint16_t v_zero) {
    int16_t horizontal             = analogReadPin(horizontalPin);
    int16_t vertical               = analogReadPin(verticalPin);
    bool    update_joystick_status = false;
    if (config.mirror_axes) {
        horizontal = 1023 - horizontal;
        vertical   = 1023 - vertical;
        h_zero     = 1023 - h_zero;
        v_zero     = 1023 - v_zero;
    }

    if (horizontal < h_min) horizontal = h_min;
    if (horizontal > h_max) horizontal = h_max;
    if (vertical < v_min) vertical = v_min;
    if (vertical > v_max) vertical = v_max;
    int8_t horizontal_mapped = 0;
    int8_t vertical_mapped   = 0;
    bool   is_split_h_axis   = false;
    bool   is_split_v_axis   = false;
    // h axis
    if (config.h_enabled) {
        if (config.split_h_axis) {
            if (horizontal >= h_zero + deadzone) {
                horizontal_mapped = mapToRange_analog(horizontal, h_zero + deadzone, h_max, -128, 127);
                is_split_h_axis   = true;
            } else if (horizontal <= h_zero - deadzone) {
                horizontal_mapped = mapToRange_analog(horizontal, h_zero - deadzone, h_min, -128, 127);
            } else {
                horizontal_mapped = -128;
            }
        } else {
            if (horizontal >= h_zero + deadzone) {
                horizontal_mapped = mapToRange_analog(horizontal, h_zero + deadzone, h_max, 0, 127);
            } else if (horizontal <= h_zero - deadzone) {
                horizontal_mapped = mapToRange_analog(horizontal, h_min, h_zero - deadzone, -128, 0);
            }
        }
        if (config.h_pos_button == -1 && (joystick_status.axes[config.h_axis] != horizontal_mapped || joystick_status.axes[config.h_axis_split] != horizontal_mapped)) {
            if (!config.h_axis_split) {
                joystick_status.axes[config.h_axis] = horizontal_mapped;
                update_joystick_status              = true;
            } else if (is_split_h_axis) {
                joystick_status.axes[config.h_axis]       = -128;
                joystick_status.axes[config.h_axis_split] = horizontal_mapped;
                update_joystick_status                    = true;
            } else {
                joystick_status.axes[config.h_axis_split] = -128;
                joystick_status.axes[config.h_axis]       = horizontal_mapped;
                update_joystick_status                    = true;
            }
        } else if (horizontal_mapped > config.button_cutoff) {
            register_joystick_button(config.h_pos_button);
        } else if (horizontal_mapped < -config.button_cutoff) {
            register_joystick_button(config.h_neg_button);
        } else {
            unregister_joystick_button(config.h_neg_button);
            unregister_joystick_button(config.h_pos_button);
        }
    }
    // v axis
    if (config.v_enabled) {
        if (config.split_v_axis) {
            if (vertical >= v_zero + deadzone) {
                vertical_mapped = mapToRange_analog(vertical, v_zero + deadzone, v_max, -128, 127);
                is_split_v_axis = true;
            } else if (vertical <= v_zero - deadzone) {
                vertical_mapped = mapToRange_analog(vertical, v_min, v_zero - deadzone, -128, 127);
            } else {
                vertical_mapped = -128;
            }
        } else {
            if (vertical >= v_zero + deadzone) {
                vertical_mapped = mapToRange_analog(vertical, v_zero + deadzone, v_max, 0, 127);
            } else if (vertical <= v_zero - deadzone) {
                vertical_mapped = mapToRange_analog(vertical, v_min, v_zero - deadzone, -128, 0);
            }
        }
        if (config.v_pos_button == -1 && (joystick_status.axes[config.v_axis] != vertical_mapped || joystick_status.axes[config.v_axis_split] != vertical_mapped)) {
            if (!config.v_axis_split) {
                joystick_status.axes[config.v_axis] = vertical_mapped;
                update_joystick_status              = true;
            } else if (is_split_v_axis) {
                joystick_status.axes[config.v_axis]       = -128;
                joystick_status.axes[config.v_axis_split] = vertical_mapped;
                update_joystick_status                    = true;
            } else {
                joystick_status.axes[config.v_axis_split] = -128;
                joystick_status.axes[config.v_axis]       = vertical_mapped;
                update_joystick_status                    = true;
            }
        } else if (vertical_mapped > config.button_cutoff) {
            register_joystick_button(config.v_pos_button);
        } else if (vertical_mapped < -config.button_cutoff) {
            register_joystick_button(config.v_neg_button);
        } else {
            unregister_joystick_button(config.v_neg_button);
            unregister_joystick_button(config.v_pos_button);
        }
    }
    if (update_joystick_status) {
        joystick_status.status |= JS_UPDATED;
    }
#ifdef CONSOLE_ENABLE
    if (timer_elapsed(scan_timer) > 200 && config.h_axis == 0) {
        // uprintf("h: %d\t", horizontal);
        // uprintf("h: %d\t\t", horizontal_mapped);
        // uprintf("v: %d\t", vertical);
        // uprintf("v: %d\n", vertical_mapped);
        scan_timer = timer_read();
    }
#endif
    return (struct joystick_data){.success = true, .horizontal = horizontal_mapped, .vertical = vertical_mapped, .buttonCurrent = 0};
}

uint8_t scanJoystick_wasd(int8_t joystickAddr, bool isLeft) {
    uint8_t nDevices = 0;
    // joystick_data joystickData = scanJoystick(joystickAddr);
    // implement layer changes based on joystick position
    return nDevices;
}

#ifdef I2C_ENABLE
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
#    ifdef CONSOLE_ENABLE
        bool axesChanged = false;
#    endif
        if (joystick_status.axes[axis1] != joystickData.horizontal) {
            joystick_status.axes[axis1] = joystickData.horizontal;
            joystick_status.status |= JS_UPDATED;
#    ifdef CONSOLE_ENABLE
            axesChanged = true;
#    endif
        }
        if (joystick_status.axes[axis2] != joystickData.vertical) {
            joystick_status.axes[axis2] = joystickData.vertical;
            joystick_status.status |= JS_UPDATED;
#    ifdef CONSOLE_ENABLE
            axesChanged = true;
#    endif
        }
#    ifdef CONSOLE_ENABLE
        if (axesChanged) {
            // uprintf("%d h: %d \t\t %d v: %d\n", joystickAddr, joystickData.horizontal, joystickAddr, joystickData.vertical);
        }
#    endif
        nDevices++;
    }
    return nDevices;
}
#endif

uint16_t L_H_MIN    = 1023;
uint16_t L_H_MAX    = 0;
uint16_t L_V_MIN    = 1023;
uint16_t L_V_MAX    = 0;
uint16_t L_DEADZONE = 10;
uint16_t L_H_ZERO   = 512;
uint16_t L_V_ZERO   = 512;
uint16_t R_H_MIN    = 1023;
uint16_t R_H_MAX    = 0;
uint16_t R_V_MIN    = 1023;
uint16_t R_V_MAX    = 0;
uint16_t R_DEADZONE = 30;
uint16_t R_H_ZERO   = 512;
uint16_t R_V_ZERO   = 512;

void scanJoysticks(struct analog_config left_analog_config, struct analog_config right_analog_config) {
    scanJoystick_controller_analog(LEFT_ANALOG_HORIZONTAL, LEFT_ANALOG_VERTICAL, left_analog_config, L_H_MIN + L_DEADZONE, L_H_MAX - L_DEADZONE, L_V_MIN + L_DEADZONE, L_V_MAX - L_DEADZONE, L_DEADZONE, L_H_ZERO, L_V_ZERO);
    scanJoystick_controller_analog(RIGHT_ANALOG_HORIZONTAL, RIGHT_ANALOG_VERTICAL, right_analog_config, R_H_MIN + R_DEADZONE, R_H_MAX - R_DEADZONE, R_V_MIN + R_DEADZONE, R_V_MAX - R_DEADZONE, R_DEADZONE, R_H_ZERO, R_V_ZERO);
}

// void scanJoysticksWithReplace(uint16_t replaceRYPos, uint16_t replaceRYNeg, bool mirrorLeft, bool mirrorRight) {
//     scanJoystick_controller_analog(LEFT_ANALOG_HORIZONTAL, LEFT_ANALOG_VERTICAL, 0, 1, L_H_MIN + L_DEADZONE, L_H_MAX - L_DEADZONE, L_V_MIN + L_DEADZONE, L_V_MAX - L_DEADZONE, L_DEADZONE, L_H_ZERO, L_V_ZERO, mirrorLeft, 0, 0);
//     scanJoystick_controller_analog(RIGHT_ANALOG_HORIZONTAL, RIGHT_ANALOG_VERTICAL, 2, 3, R_H_MIN + R_DEADZONE, R_H_MAX - R_DEADZONE, R_V_MIN + R_DEADZONE, R_V_MAX - R_DEADZONE, R_DEADZONE, R_H_ZERO, R_V_ZERO, mirrorRight, replaceRYPos, replaceRYNeg);
// }

void calibrateJoysticks(bool mirrorLeft, bool mirrorRight) {
    if (DO_CALIBRATE_ANALOG) {
        uint16_t calibrateTime = timer_elapsed(calibrateTimer);
        if (calibrateTime > 10000) {
            uprintf("Joystick calibration complete\n");
            uprintf("L_H_ZERO: %d\n", L_H_ZERO);
            uprintf("L_V_ZERO: %d\n", L_V_ZERO);
            uprintf("R_H_ZERO: %d\n", R_H_ZERO);
            uprintf("R_V_ZERO: %d\n", R_V_ZERO);
            uprintf("L_H_MIN: %d\n", L_H_MIN);
            uprintf("L_H_MAX: %d\n", L_H_MAX);
            uprintf("L_V_MIN: %d\n", L_V_MIN);
            uprintf("L_V_MAX: %d\n", L_V_MAX);
            uprintf("R_H_MIN: %d\n", R_H_MIN);
            uprintf("R_H_MAX: %d\n", R_H_MAX);
            uprintf("R_V_MIN: %d\n", R_V_MIN);
            uprintf("R_V_MAX: %d\n", R_V_MAX);
            calibrateTimer = 0;
        }
        int16_t leftHorizontal  = mirrorLeft ? 1023 - analogReadPin(LEFT_ANALOG_HORIZONTAL) : analogReadPin(LEFT_ANALOG_HORIZONTAL);
        int16_t leftVertical    = mirrorLeft ? 1023 - analogReadPin(LEFT_ANALOG_VERTICAL) : analogReadPin(LEFT_ANALOG_VERTICAL);
        int16_t rightHorizontal = mirrorRight ? 1023 - analogReadPin(RIGHT_ANALOG_HORIZONTAL) : analogReadPin(RIGHT_ANALOG_HORIZONTAL);
        int16_t rightVertical   = mirrorRight ? 1023 - analogReadPin(RIGHT_ANALOG_VERTICAL) : analogReadPin(RIGHT_ANALOG_VERTICAL);
        if (calibrateTime < 100) {
            // set zero values
            L_H_ZERO = leftHorizontal;
            L_V_ZERO = leftVertical;
            R_H_ZERO = rightHorizontal;
            R_V_ZERO = rightVertical;
        } else {
            // set min and max values
            if (leftHorizontal < L_H_MIN) {
                L_H_MIN = leftHorizontal;
                uprintf("L_H_MIN: %d\n", L_H_MIN);
            }
            if (leftHorizontal > L_H_MAX) {
                L_H_MAX = leftHorizontal;
                uprintf("L_H_MAX: %d\n", L_H_MAX);
            }
            if (leftVertical < L_V_MIN) {
                L_V_MIN = leftVertical;
                uprintf("L_V_MIN: %d\n", L_V_MIN);
            }
            if (leftVertical > L_V_MAX) {
                L_V_MAX = leftVertical;
                uprintf("L_V_MAX: %d\n", L_V_MAX);
            }
            if (rightHorizontal < R_H_MIN) {
                R_H_MIN = rightHorizontal;
                uprintf("R_H_MIN: %d\n", R_H_MIN);
            }
            if (rightHorizontal > R_H_MAX) {
                R_H_MAX = rightHorizontal;
                uprintf("R_H_MAX: %d\n", R_H_MAX);
            }
            if (rightVertical < R_V_MIN) {
                R_V_MIN = rightVertical;
                uprintf("R_V_MIN: %d\n", R_V_MIN);
            }
            if (rightVertical > R_V_MAX) {
                R_V_MAX = rightVertical;
                uprintf("R_V_MAX: %d\n", R_V_MAX);
            }
        }
    }
}
