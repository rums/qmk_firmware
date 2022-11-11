/* Copyright 2022 Micah Wine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "quantum.h"

/* This is a shortcut to help you visually see your layout.
 *
 * The first section contains all of the arguments representing the physical
 * layout of the board and position of the keys.
 *
 * The second converts the arguments into a two-dimensional array which
 * represents the switch matrix.
 */

#define ___ KC_NO

/*
#define LAYOUT_split_3x5_2( \
    L00, L01, L02, L03, L04,   R00, R01, R02, R03, R04, \
    L10, L11, L12, L13, L14,   R10, R11, R12, R13, R14, \
    L20, L21, L22, L23, L24,   R20, R21, R22, R23, R24, \
              L32, L33,        R32, R33 \
) { \
    { L00, L01, L02, L03, L04 }, \
    { L10, L11, L12, L13, L14 }, \
    { L20, L21, L22, L23, L24 }, \
    { ___, ___, L32, L33, ___ }, \
    { R00, R01, R02, R03, R04 }, \
    { R10, R11, R12, R13, R14 }, \
    { R20, R21, R22, R23, R24 }, \
    { ___, ___, R32, R33, ___ }  \
}

#define LAYOUT_split_3x5_2_rt( \
    L00, L01, L02, L03, L04,             R02, R03, R04, \
    L10, L11, L12, L13, L14,             R12, R13, R14, \
    L20, L21, L22, L23, L24,             R22, R23, R24, \
              L32, L33,                  R32, R33 \
) { \
    { L00, L01, L02, L03, L04 }, \
    { L10, L11, L12, L13, L14 }, \
    { L20, L21, L22, L23, L24 }, \
    { ___, ___, L32, L33, ___ }, \
    { ___, ___, R02, R03, R04 }, \
    { ___, ___, R12, R13, R14 }, \
    { ___, ___, R22, R23, R24 }, \
    { ___, ___, R32, R33, ___ }  \
}

#define LAYOUT_split_3x3_2( \
    L00, L01, L02,   R00, R01, R02, \
    L10, L11, L12,   R10, R11, R12, \
    L20, L21, L22,   R20, R21, R22, \
         L31, L32,   R30, R31       \
) { \
    { L00, L01, L02 }, \
    { L10, L11, L12 }, \
    { L20, L21, L22 }, \
    { ___, L31, L32 }, \
    { R00, R01, R02 }, \
    { R10, R11, R12 }, \
    { R20, R21, R22 }, \
    { R30, R31, ___ }  \
}
*/

// single keyboard with 4 rows, 6 columns. bottom row has 4 columns. 22 keys total.
#define LAYOUT_4x6_4( \
    L00, L01, L02, L03, L04, L05, \
    L10, L11, L12, L13, L14, L15, \
    L20, L21, L22, L23, L24, L25, \
         L31, L32, L33, L34       \
) { \
    { L03, L04, L05, L00, L01, L02 }, \
    { L13, L14, L15, L10, L11, L12 }, \
    { L23, L24, L25, L20, L21, L22 }, \
    { L33, L34, ___, ___, L31, L32 }  \
}

#define LAYOUT_4x6( \
    L00, L01, L02, L32, L33, L03, L04, L05, \
    L10, L11, L12,           L13, L14, L15, \
    L20, L21, L22,           L23, L24, L25, \
    L30, L31,                    L34, L35   \
) { \
    { L00, L01, L02, L03, L04, L05 }, \
    { L10, L11, L12, L13, L14, L15 }, \
    { L20, L21, L22, L23, L24, L25 }, \
    { L30, L31, L32, L33, L34, L35 }  \
}
#define LAYOUT LAYOUT_4x6
