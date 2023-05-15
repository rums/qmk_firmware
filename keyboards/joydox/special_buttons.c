#include QMK_KEYBOARD_H
#include "special_buttons.h"

uint16_t wd_timer;
bool     wd_first  = false;
bool     wd_second = false;
bool     wd_stop   = false;
struct two_button_state powerslide_state = {false, false, false, false};

void handle_wd(void) {
    if (wd_first) {
        register_joystick_button(BUTTON(XBOX_A));
        wd_first  = false;
        wd_second = true;
        wd_stop   = true;
        wd_timer  = timer_read();
    } else if (wd_second && (timer_elapsed(wd_timer) > 65)) {
        register_joystick_button(BUTTON(XBOX_A));
        wd_second = false;
        wd_stop   = true;
    } else if (wd_stop) {
        unregister_joystick_button(BUTTON(XBOX_A));
        wd_stop = false;
    }
}

bool wd_manual_first = false;
bool wd_manual_second = false;
bool wd_manual_stop = false;
void handle_wd_manual(void) {
    if (wd_manual_first) {
        uprintf("wd_manual_first\n");
        register_joystick_button(BUTTON(XBOX_A));
        wd_manual_first = false;
        wd_manual_stop = true;
        wd_timer = timer_read();
    }
    else if (wd_manual_second) {
        uprintf("wd_manual_second\n");
        register_joystick_button(BUTTON(XBOX_A));
        wd_manual_second = false;
        wd_manual_stop = true;
        wd_timer = timer_read();
    }
    else if (wd_manual_stop && (timer_elapsed(wd_timer) > 10)) {
        unregister_joystick_button(BUTTON(XBOX_A));
        wd_manual_stop = false;
    }
}

bool special_powerslide_pressed = false;
bool special_powerslide_released = false;
bool powerslide_pressed = false;
bool powerslide_released = false;
bool air_left_pressed = false;
bool air_right_pressed = false;
bool air_roll_pressed = false;

void handle_air_roll_state(void) {
    if (air_roll_pressed) {
        register_joystick_button(BUTTON(XBOX_B));
        unregister_joystick_button(BUTTON(XBOX_LS));
        unregister_joystick_button(BUTTON(XBOX_RS));
    }
    else if (air_left_pressed && air_right_pressed) {
        register_joystick_button(BUTTON(XBOX_B));
        unregister_joystick_button(BUTTON(XBOX_LS));
        unregister_joystick_button(BUTTON(XBOX_RS));
    }
    else if (air_left_pressed) {
        register_joystick_button(BUTTON(XBOX_LS));
        unregister_joystick_button(BUTTON(XBOX_RS));
        unregister_joystick_button(BUTTON(XBOX_B));
    }
    else if (air_right_pressed) {
        register_joystick_button(BUTTON(XBOX_RS));
        unregister_joystick_button(BUTTON(XBOX_LS));
        unregister_joystick_button(BUTTON(XBOX_B));
    }
    else {
        unregister_joystick_button(BUTTON(XBOX_LS));
        unregister_joystick_button(BUTTON(XBOX_RS));
        unregister_joystick_button(BUTTON(XBOX_B));
    }
}

void handle_special_powerslide(void) {
    if (special_powerslide_pressed) {
        // if air roll left or right are pressed, don't air roll
        if (joystick_status.buttons[BUTTON(XBOX_RS) / 8] & (1 << (BUTTON(XBOX_RS) % 8)) || joystick_status.buttons[BUTTON(XBOX_LS) / 8] & (1 << (BUTTON(XBOX_LS) % 8))) {
            register_joystick_button(BUTTON(XBOX_LB));
            unregister_joystick_button(BUTTON(XBOX_B));
        } else {
            register_joystick_button(BUTTON(XBOX_LB));
            register_joystick_button(BUTTON(XBOX_B));
        }
    } else if (special_powerslide_released) {
        unregister_joystick_button(BUTTON(XBOX_LB));
        unregister_joystick_button(BUTTON(XBOX_B));
        special_powerslide_released = false;
    }
}

void handle_powerslide(void) {
    if (powerslide_state.button1_pressed || powerslide_state.button2_pressed) {
        register_joystick_button(BUTTON(XBOX_LB));
    } else if ((powerslide_state.button1_released || powerslide_state.button2_released) && (powerslide_state.button1_released || !powerslide_state.button1_pressed) && (powerslide_state.button2_released || !powerslide_state.button2_pressed)) {
        unregister_joystick_button(BUTTON(XBOX_LB));
        powerslide_state.button1_released = false;
        powerslide_state.button2_released = false;
    }
}
