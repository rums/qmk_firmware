#pragma once

#include "matrix.h"

#define ROWS_PER_HAND (MATRIX_ROWS / 2)
#define TRIGGER_ENABLE yes

#if defined(USE_I2C)

typedef struct _I2C_slave_buffer_t {
#    ifndef DISABLE_SYNC_TIMER
    uint32_t sync_timer;
#    endif
#    ifdef SPLIT_TRANSPORT_MIRROR
    matrix_row_t mmatrix[ROWS_PER_HAND];
#    endif
    matrix_row_t smatrix[ROWS_PER_HAND];
#    ifdef SPLIT_MODS_ENABLE
    uint8_t real_mods;
    uint8_t weak_mods;
#        ifndef NO_ACTION_ONESHOT
    uint8_t oneshot_mods;
#        endif
#    endif
#    ifdef BACKLIGHT_ENABLE
    uint8_t backlight_level;
#    endif
#    if defined(RGBLIGHT_ENABLE) && defined(RGBLIGHT_SPLIT)
    rgblight_syncinfo_t rgblight_sync;
#    endif
#    ifdef ENCODER_ENABLE
    uint8_t encoder_state[NUMBER_OF_ENCODERS];
#    endif
#    ifdef TRIGGER_ENABLE
    uint8_t trigger_state;
#    endif
#    ifdef WPM_ENABLE
    uint8_t current_wpm;
#    endif
#    if defined(LED_MATRIX_ENABLE) && defined(LED_MATRIX_SPLIT)
    led_eeconfig_t led_matrix;
    bool           led_suspend_state;
#    endif
#    if defined(RGB_MATRIX_ENABLE) && defined(RGB_MATRIX_SPLIT)
    rgb_config_t rgb_matrix;
    bool         rgb_suspend_state;
#    endif
} I2C_slave_buffer_t;
#endif

void transport_master_init(void);
void transport_slave_init(void);

// returns false if valid data not received from slave
bool transport_master(matrix_row_t master_matrix[], matrix_row_t slave_matrix[]);
void transport_slave(matrix_row_t master_matrix[], matrix_row_t slave_matrix[]);
