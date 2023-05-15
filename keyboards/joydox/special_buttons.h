#define BUTTON(x) (x - JS_BUTTON0)

enum controller_mappings { XBOX_A = JS_BUTTON4, XBOX_B = JS_BUTTON20, XBOX_X = JS_BUTTON0, XBOX_Y = JS_BUTTON10, XBOX_LB = JS_BUTTON3, XBOX_RB = JS_BUTTON5, XBOX_BACK = JS_BUTTON11, XBOX_START = JS_BUTTON15, XBOX_LS = JS_BUTTON2, XBOX_RS = JS_BUTTON9, XBOX_UP = JS_BUTTON18, XBOX_DOWN = JS_BUTTON19, XBOX_LEFT = JS_BUTTON12, XBOX_RIGHT = JS_BUTTON13 };

struct two_button_state {
    bool button1_pressed;
    bool button2_pressed;
    bool button1_released;
    bool button2_released;
};

extern uint16_t wd_timer;
extern bool     wd_first;
extern bool     wd_second;
extern bool     wd_stop;
extern bool    wd_manual_first;
extern bool    wd_manual_second;
extern bool    wd_manual_stop;

extern bool special_powerslide_pressed;
extern bool special_powerslide_released;
extern struct two_button_state powerslide_state;
extern bool air_left_pressed;
extern bool air_right_pressed;
extern bool air_roll_pressed;

void handle_wd(void);
void handle_wd_manual(void);
void handle_special_powerslide(void);
void handle_powerslide(void);
void handle_air_roll_state(void);
