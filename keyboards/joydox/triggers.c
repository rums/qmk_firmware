#include QMK_KEYBOARD_H
#include <string.h>
#include "triggers.h"
#include "analog.h"
#include "math.h"

uint8_t ignoreFrames = 0;

uint16_t L_TRIGGER_MIN  = 1023;
uint16_t L_TRIGGER_MAX  = 0;
uint16_t L_TRIGGER_ZERO = 512;
uint16_t R_TRIGGER_MIN  = 1023;
uint16_t R_TRIGGER_MAX  = 0;
uint16_t R_TRIGGER_ZERO = 512;

#define QWIIC_TRIGGER_LEFT_ADDR (0x64 << 1)

#ifdef SPLIT_KEYBOARD
static uint8_t trigger_value = 0;

void trigger_state_raw(uint8_t slave_state) {
    memcpy(&slave_state, &trigger_value, sizeof(uint8_t));
}

void trigger_update_raw(uint8_t slave_state) {
    if (!is_keyboard_master()) {
        trigger_value = slave_state;
    }
}
#endif

int16_t mapToRange_trigger(int16_t value, int16_t min, int16_t max, int8_t min_out, int8_t max_out) {
    double slope = 1.0 * (max_out - min_out) / (max - min);
    return (int16_t)(slope * (value - min) + min_out);
    // return MIN(ceil(slope * (value - min) + min_out), max_out);
}

// int16_t debugTimer = 0;
uint16_t getTriggerValue(uint8_t triggerPin, uint16_t axisZero, uint16_t minInput, uint16_t maxInput, uint8_t minOutput, uint8_t maxOutput, bool mirror, uint16_t deadzone) {
    int32_t rawVal = analogReadPin(triggerPin);
    // if (timer_elapsed(debugTimer) > 1000) {
    //     uprintf("triggerPin: %d, rawVal: %d\n", triggerPin, rawVal);
    // }
    if (mirror) {
        rawVal = 1023 - rawVal;
    }
    if (rawVal < minInput) {
        rawVal = minInput;
    }
    if (rawVal > maxInput) {
        rawVal = maxInput;
    }
    // map rawVal from input range to output range
    int16_t rangedVal = mapToRange_trigger(rawVal, minInput, maxInput, minOutput, maxOutput);
    // if (timer_elapsed(debugTimer) > 200) {
    //     uprintf("rangedVal: %d\n", rangedVal);
    //     debugTimer = timer_read();
    // }
    return rangedVal;
}

void wasdTriggers(int32_t rawVal, uint16_t axisZero, uint16_t minInput, uint16_t maxInput, bool triggerToggle) {
    if (rawVal > axisZero + 1) {
        uint8_t minOutput = 0;
        uint8_t maxOutput = 127;
        if (rawVal > maxInput) {
            rawVal = maxInput;
        }
        // map rawVal from input range tooutput range
        double  slope     = 1.0 * (maxOutput - minOutput) / (maxInput - minInput);
        int16_t rangedVal = MIN(ceil(slope * (rawVal - minInput) + minOutput), maxOutput);
        if (rangedVal == maxOutput && !triggerToggle) {
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

int16_t prev_right_trigger = 0;
int16_t smoothAnalog(int32_t readPin) {
    if (abs(analogReadPin(readPin) - prev_right_trigger) > 30) {
        return prev_right_trigger;
    }
    const uint8_t sampleCount = 20;
    int16_t       rawVals[20] = {0};
    int16_t       avgVal      = 0;
    for (uint8_t i = 0; i < sampleCount; i++) {
        rawVals[i] = analogReadPin(readPin);
        avgVal += rawVals[i];
    }
    avgVal /= sampleCount;
    if (avgVal > 0) {
        // recalculate mean, excluding values that are too far away from the mean
        int16_t newAvgVal = 0;
        for (uint8_t i = 0; i < sampleCount; i++) {
            if (abs(rawVals[i] - avgVal) < 10) {
                newAvgVal += rawVals[i];
            }
        }
        newAvgVal /= sampleCount;
        prev_right_trigger = newAvgVal;
        return newAvgVal;
    }
    prev_right_trigger = avgVal;
    return avgVal;
}

void scanTriggers(bool wasdMode, uint8_t axis1, uint8_t axis2, uint8_t tapAxis, bool useTapAxis1, bool useTapAxis2, bool mirrorLeft, bool mirrorRight) {
    uint16_t leftDeadzone       = 10;
    uint16_t rightDeadzone      = 10;
    int16_t  leftTrigVal        = getTriggerValue(LEFT_TRIGGER_PIN, L_TRIGGER_ZERO, L_TRIGGER_MIN + leftDeadzone, L_TRIGGER_MAX, -128, 127, mirrorLeft, leftDeadzone);
    int16_t  rightTrigVal       = getTriggerValue(RIGHT_TRIGGER_PIN, R_TRIGGER_ZERO, R_TRIGGER_MIN + rightDeadzone, R_TRIGGER_MAX, -128, 127, mirrorRight, rightDeadzone);
    joystick_status.axes[axis1] = leftTrigVal;
    joystick_status.axes[axis2] = rightTrigVal;
    // if both are pressed, ignore tap axis
    if (tapAxis != -1) {
        if ((!useTapAxis1 && useTapAxis2) || (useTapAxis1 && !useTapAxis2)) {
            int16_t leftTapVal            = mapToRange_trigger(leftTrigVal, -128, 127, 0, useTapAxis1 ? 127 : -128);
            int16_t rightTapVal           = mapToRange_trigger(rightTrigVal, -128, 127, 0, useTapAxis1 ? 127 : -128);
            int16_t maxTapVal             = useTapAxis1 ? MAX(leftTapVal, rightTapVal) : MIN(leftTapVal, rightTapVal);
            joystick_status.axes[tapAxis] = maxTapVal;
        } else {
            joystick_status.axes[tapAxis] = 0;
        }
    }
    joystick_status.status |= JS_UPDATED;
}

void calibrateTriggers(bool mirrorLeft, bool mirrorRight) {
    if (DO_CALIBRATE_ANALOG) {
        uint16_t calibrateTime = timer_elapsed(calibrateTimer);
        if (calibrateTime > 10000) {
            DO_CALIBRATE_ANALOG = false;
            uprintf("Trigger calibration complete\n");
            uprintf("L_TRIGGER_MIN: %d\n", L_TRIGGER_MIN);
            uprintf("L_TRIGGER_MAX: %d\n", L_TRIGGER_MAX);
            uprintf("L_TRIGGER_ZERO: %d\n", L_TRIGGER_ZERO);
            uprintf("R_TRIGGER_MIN: %d\n", R_TRIGGER_MIN);
            uprintf("R_TRIGGER_MAX: %d\n", R_TRIGGER_MAX);
            uprintf("R_TRIGGER_ZERO: %d\n", R_TRIGGER_ZERO);
            calibrateTimer = 0;
        }
        int16_t leftTrigger  = analogReadPin(LEFT_TRIGGER_PIN);
        int16_t rightTrigger = analogReadPin(RIGHT_TRIGGER_PIN);
        if (mirrorLeft) {
            leftTrigger = 1023 - leftTrigger;
        }
        if (mirrorRight) {
            rightTrigger = 1023 - rightTrigger;
        }
        if (calibrateTime < 100) {
            // set zero values
            L_TRIGGER_ZERO = leftTrigger;
            R_TRIGGER_ZERO = rightTrigger;
            L_TRIGGER_MIN  = leftTrigger;
            R_TRIGGER_MIN  = rightTrigger;
        } else {
            if (leftTrigger > L_TRIGGER_MAX) {
                L_TRIGGER_MAX = leftTrigger;
                uprintf("L_TRIGGER_MAX: %d\n", L_TRIGGER_MAX);
            }
            if (rightTrigger > R_TRIGGER_MAX) {
                R_TRIGGER_MAX = rightTrigger;
                uprintf("R_TRIGGER_MAX: %d\n", R_TRIGGER_MAX);
            }
        }
    }
}
