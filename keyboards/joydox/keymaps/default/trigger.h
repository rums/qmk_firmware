#pragma once

#include "quantum.h"

#ifdef SPLIT_KEYBOARD
void trigger_state_raw(uint8_t* slave_state);
void trigger_update_raw(uint8_t* slave_state);
#endif
