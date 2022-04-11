#include <string.h>
#include QMK_KEYBOARD_H

#ifdef SPLIT_KEYBOARD
static uint8_t trigger_value = 0;

void trigger_state_raw(uint8_t slave_state) { memcpy(&slave_state, &trigger_value, sizeof(uint8_t)); }

void trigger_update_raw(uint8_t slave_state) {
    if (!is_keyboard_master()) {
        trigger_value = slave_state;
    }
}
#endif
