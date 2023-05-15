struct joystick_data {
    bool    success;
    int8_t  horizontal;
    int8_t  vertical;
    uint8_t buttonCurrent;
};

extern bool DO_CALIBRATE_ANALOG;
extern uint16_t calibrateTimer;
extern uint16_t scan_timer;

void scanJoysticks(uint16_t replaceRYPos, uint16_t replaceRYNeg, bool mirrorLeft, bool mirrorRight);
void calibrateJoysticks(bool mirrorLeft, bool mirrorRight);
