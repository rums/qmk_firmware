struct joystick_data {
    bool    success;
    int8_t  horizontal;
    int8_t  vertical;
    uint8_t buttonCurrent;
};

struct analog_config {
    bool h_enabled;
    bool v_enabled;
    uint8_t  h_axis;
    uint8_t  v_axis;
    bool split_h_axis;
    uint8_t h_axis_split;
    bool split_v_axis;
    uint8_t v_axis_split;
    bool mirror_axes;
    int16_t h_pos_button;
    int16_t h_neg_button;
    int16_t v_pos_button;
    int16_t v_neg_button;
    uint8_t  button_cutoff;
};

extern bool DO_CALIBRATE_ANALOG;
extern uint16_t calibrateTimer;
extern uint16_t scan_timer;

void scanJoysticks(struct analog_config left_analog_config, struct analog_config right_analog_config);
void calibrateJoysticks(bool mirrorLeft, bool mirrorRight);
