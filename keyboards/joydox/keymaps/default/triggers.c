#include QMK_KEYBOARD_H
#include <string.h>
#include "triggers.h"
#include "analog.h"

uint8_t ignoreFrames = 0;

#define QWIIC_TRIGGER_LEFT_ADDR (0x64 << 1)

#ifdef SPLIT_KEYBOARD
static uint8_t trigger_value = 0;

void trigger_state_raw(uint8_t slave_state) { memcpy(&slave_state, &trigger_value, sizeof(uint8_t)); }

void trigger_update_raw(uint8_t slave_state) {
    if (!is_keyboard_master()) {
        trigger_value = slave_state;
    }
}
#endif

int16_t mapToRange_trigger(int16_t value, int16_t min, int16_t max, int8_t min_out, int8_t max_out) {
    double slope = 1.0 * (max_out - min_out) / (max - min);
    return MIN(ceil(slope * (value - min) + min_out), max_out);
}

void controllerTriggers(uint8_t axis, int32_t rawVal, uint16_t axisZero, uint16_t minInput, uint16_t maxInput) {
    bool jsUpdated = false;
    if (rawVal > axisZero + 1) {
        if (rawVal > maxInput) {
            rawVal = maxInput;
        }
        uint8_t minOutput = 0;
        uint8_t maxOutput = 127;
        // map rawVal from input range to output range
        int16_t rangedVal = mapToRange_trigger(rawVal, minInput, maxInput, minOutput, maxOutput);
        if (joystick_status.axes[axis] != rangedVal) {
            joystick_status.axes[axis] = rangedVal;
            jsUpdated                  = true;
        }
    } else {
        int16_t rangedVal = 0;
        if (rangedVal != joystick_status.axes[axis]) {
            if (abs(rangedVal - joystick_status.axes[axis]) < 20 || ignoreFrames == 10) {
                joystick_status.axes[axis] = rangedVal;
                jsUpdated                  = true;
                ignoreFrames = 0;
            } else {
                ignoreFrames += 1;
            }
        }
    }
    if (jsUpdated) {
        joystick_status.status |= JS_UPDATED;
    }
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
    int16_t rawVals[20] = {0};
    int16_t avgVal    = 0;
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

void scanTriggers(bool wasdMode) {
    int16_t rightRawVal = analogReadPin(RIGHT_TRIGGER_PIN);
    int16_t leftRawVal = analogReadPin(LEFT_TRIGGER_PIN);
    if (wasdMode) {
        //wasdTriggers(leftRawVal, 510, 288, 510);
        //wasdTriggers(rightRawVal, 516, 528, 750);
    } else {
        leftRawVal = 1023 - leftRawVal;
        controllerTriggers(4, leftRawVal, 513, 528, 980);
        controllerTriggers(5, rightRawVal, 516, 528, 980);
    }
}
