struct joystick_data {
    bool    success;
    int8_t  horizontal;
    int8_t  vertical;
    uint8_t buttonCurrent;
};

struct analog_config {
    uint8_t  h_axis;
    uint8_t  v_axis;
    int16_t h_pos_button;
    int16_t h_neg_button;
    int16_t v_pos_button;
    int16_t v_neg_button;
};

extern bool DO_CALIBRATE_ANALOG;
extern uint16_t calibrateTimer;
extern uint16_t scan_timer;

void scanJoysticks(struct analog_config left_analog_config, struct analog_config right_analog_config, bool mirrorLeft, bool mirrorRight);
void calibrateJoysticks(bool mirrorLeft, bool mirrorRight);
