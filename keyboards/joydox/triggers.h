#pragma once

#ifdef SPLIT_KEYBOARD
void trigger_state_raw(uint8_t* slave_state);
void trigger_update_raw(uint8_t* slave_state);
#endif

extern bool DO_CALIBRATE_ANALOG;
extern uint16_t calibrateTimer;
extern bool trigger_toggle;

void scanTriggers(bool wasdMode);
void calibrateTriggers(bool mirror);
