
// SPDX-License-Identifier: MIT

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>



// Badge buttons.
// Not all buttons are implemented on every badge.
typedef enum {
    // None or unknown input.
    BSP_INPUT_NONE,

    // Navigate to the previous element/option.
    BSP_INPUT_PREV,
    // Navigate to the next element/option.
    BSP_INPUT_NEXT,

    // Keyboard/DPAD left.
    BSP_INPUT_LEFT,
    // Keyboard/DPAD right.
    BSP_INPUT_RIGHT,
    // Keyboard/DPAD up.
    BSP_INPUT_UP,
    // Keyboard/DPAD down.
    BSP_INPUT_DOWN,

    // Keyboard home.
    BSP_INPUT_HOME,
    // Keyboard end.
    BSP_INPUT_END,
    // Keyboard page up.
    BSP_INPUT_PGUP,
    // Keyboard page down.
    BSP_INPUT_PGDN,

    // A / Accept button.
    BSP_INPUT_ACCEPT,
    // B / Back button.
    BSP_INPUT_BACK,

    // Keyboard spacebar.
    BSP_INPUT_SPACE = ' ',

    // Keyboard button `~
    BSP_INPUT_KB_TILDA     = '`',
    // Keyboard button 1
    BSP_INPUT_KB_1         = '1',
    // Keyboard button 2
    BSP_INPUT_KB_2         = '2',
    // Keyboard button 3
    BSP_INPUT_KB_3         = '3',
    // Keyboard button 4
    BSP_INPUT_KB_4         = '4',
    // Keyboard button 5
    BSP_INPUT_KB_5         = '5',
    // Keyboard button 6
    BSP_INPUT_KB_6         = '6',
    // Keyboard button 7
    BSP_INPUT_KB_7         = '7',
    // Keyboard button 8
    BSP_INPUT_KB_8         = '8',
    // Keyboard button 9
    BSP_INPUT_KB_9         = '9',
    // Keyboard button 0
    BSP_INPUT_KB_0         = '0',
    // Keyboard button -_
    BSP_INPUT_KB_MINUS     = '-',
    // Keyboard button =+
    BSP_INPUT_KB_EQUALS    = '=',
    // Keyboard button [{
    BSP_INPUT_KB_LBRAC     = '[',
    // Keyboard button ]}
    BSP_INPUT_KB_RBRAC     = ']',
    // Keyboard button \|
    BSP_INPUT_KB_BACKSLASH = '\\',
    // Keyboard button ;:
    BSP_INPUT_KB_SEMICOLON = ';',
    // Keyboard button '"
    BSP_INPUT_KB_QUOTE     = '\'',
    // Keyboard button ,<
    BSP_INPUT_KB_COMMA     = ',',
    // Keyboard button .>
    BSP_INPUT_KB_DOT       = '.',
    // Keyboard button /?
    BSP_INPUT_KB_SLASH     = '/',

    // Keyboard button A
    BSP_INPUT_KB_A = 'A',
    // Keyboard button B
    BSP_INPUT_KB_B = 'B',
    // Keyboard button C
    BSP_INPUT_KB_C = 'C',
    // Keyboard button D
    BSP_INPUT_KB_D = 'D',
    // Keyboard button E
    BSP_INPUT_KB_E = 'E',
    // Keyboard button F
    BSP_INPUT_KB_F = 'F',
    // Keyboard button G
    BSP_INPUT_KB_G = 'G',
    // Keyboard button H
    BSP_INPUT_KB_H = 'H',
    // Keyboard button I
    BSP_INPUT_KB_I = 'I',
    // Keyboard button J
    BSP_INPUT_KB_J = 'J',
    // Keyboard button K
    BSP_INPUT_KB_K = 'K',
    // Keyboard button L
    BSP_INPUT_KB_L = 'L',
    // Keyboard button M
    BSP_INPUT_KB_M = 'M',
    // Keyboard button N
    BSP_INPUT_KB_N = 'N',
    // Keyboard button O
    BSP_INPUT_KB_O = 'O',
    // Keyboard button P
    BSP_INPUT_KB_P = 'P',
    // Keyboard button Q
    BSP_INPUT_KB_Q = 'Q',
    // Keyboard button R
    BSP_INPUT_KB_R = 'R',
    // Keyboard button S
    BSP_INPUT_KB_S = 'S',
    // Keyboard button T
    BSP_INPUT_KB_T = 'T',
    // Keyboard button U
    BSP_INPUT_KB_U = 'U',
    // Keyboard button V
    BSP_INPUT_KB_V = 'V',
    // Keyboard button W
    BSP_INPUT_KB_W = 'W',
    // Keyboard button X
    BSP_INPUT_KB_X = 'X',
    // Keyboard button Y
    BSP_INPUT_KB_Y = 'Y',
    // Keyboard button Z
    BSP_INPUT_KB_Z = 'Z',

    // Keyboard F1
    BSP_INPUT_F1 = 0x100,
    // Keyboard F2
    BSP_INPUT_F2,
    // Keyboard F3
    BSP_INPUT_F3,
    // Keyboard F4
    BSP_INPUT_F4,
    // Keyboard F5
    BSP_INPUT_F5,
    // Keyboard F6
    BSP_INPUT_F6,
    // Keyboard F7
    BSP_INPUT_F7,
    // Keyboard F8
    BSP_INPUT_F8,
    // Keyboard F9
    BSP_INPUT_F9,
    // Keyboard F10
    BSP_INPUT_F10,
    // Keyboard F11
    BSP_INPUT_F11,
    // Keyboard F12
    BSP_INPUT_F12,

    // Numpad 0 / insert
    BSP_INPUT_NP_0,
    // Numpad 1 / end
    BSP_INPUT_NP_1,
    // Numpad 2 / down
    BSP_INPUT_NP_2,
    // Numpad 3 / page down
    BSP_INPUT_NP_3,
    // Numpad 4 / left
    BSP_INPUT_NP_4,
    // Numpad 5
    BSP_INPUT_NP_5,
    // Numpad 6 / right
    BSP_INPUT_NP_6,
    // Numpad 7 / home
    BSP_INPUT_NP_7,
    // Numpad 8 / up
    BSP_INPUT_NP_8,
    // Numpad 9 / page up
    BSP_INPUT_NP_9,
    // Numpad /
    BSP_INPUT_NP_SLASH,
    // Numpad *
    BSP_INPUT_NP_STAR,
    // Numpad -
    BSP_INPUT_NP_MINUS,
    // Numpad +
    BSP_INPUT_NP_PLUS,
    // Numpad enter
    BSP_INPUT_NP_ENTER,
    // Numpad . / delete
    BSP_INPUT_NP_PERIOD,

    // Keyboard left shift.
    BSP_INPUT_L_SHIFT = 0x200,
    // Keyboard right shift.
    BSP_INPUT_R_SHIFT,
    // Keyboard left control.
    BSP_INPUT_L_CTRL,
    // Keyboard left control.
    BSP_INPUT_R_CTRL,
    // Keyboard left alt.
    BSP_INPUT_L_ALT,
    // Keyboard right alt.
    BSP_INPUT_R_ALT,
    // Keyboard left super.
    BSP_INPUT_L_SUPER,
    // Keyboard right super.
    BSP_INPUT_R_SUPER,

    // Keyboard Fn/function key.
    BSP_INPUT_FUNCTION,
    // Keyboard enter/return key.
    BSP_INPUT_ENTER,
    // Keyboard backspace.
    BSP_INPUT_BACKSPACE,
    // Keyboard delete.
    BSP_INPUT_DELETE,
    // Keyboard escape.
    BSP_INPUT_ESCAPE,
    // Keyboard tab.
    BSP_INPUT_TAB,
    // Keyboard context menu (next to right super).
    BSP_INPUT_CTXMENU,

    // Keyboard print screen.
    BSP_INPUT_PRINTSCR,
    // Keyboard pause/break.
    BSP_INPUT_PAUSE_BREAK,
    // Keyboard insert.
    BSP_INPUT_INSERT,

    // Keyboard num lock.
    BSP_INPUT_NUM_LK,
    // Keyboard caps lock.
    BSP_INPUT_CAPS_LK,
    // Keyboard scroll lock.
    BSP_INPUT_SCROLL_LK,
} bsp_input_t;

// Types of input event.
typedef enum {
    // Button pressed.
    BSP_INPUT_EVENT_PRESS,
    // Button held down.
    BSP_INPUT_EVENT_HOLD,
    // Button released.
    BSP_INPUT_EVENT_RELEASE,
} bsp_input_event_type_t;

// Input event.
typedef struct {
    // Input event type.
    bsp_input_event_type_t type;
    // Input device ID.
    uint32_t               dev_id;
    // Input device endpoint.
    uint8_t                endpoint;
    // Input interpreted as navigation.
    bsp_input_t            nav_input;
    // Input value.
    bsp_input_t            input;
    // Input interpreted as text including DEL(0x7f), BS(\b) and ENTER(\n).
    char                   text_input;
    // Raw input value or keyboard scan code.
    int                    raw_input;
    // Active modifier keys, if any.
    uint32_t               modkeys;
} bsp_input_event_t;

/* ==== SDL2-compatible modifier keys ==== */
// Left shift pressed.
#define BSP_MODKEY_L_SHIFT   0x0001
// Right shift pressed.
#define BSP_MODKEY_R_SHIFT   0x0002
// Left control pressed.
#define BSP_MODKEY_L_CTRL    0x0040
// Right control pressed.
#define BSP_MODKEY_R_CTRL    0x0080
// Left alt pressed.
#define BSP_MODKEY_L_ALT     0x0100
// Right alt pressed.
#define BSP_MODKEY_R_ALT     0x0200
// Function key pressed.
#define BSP_MODKEY_FN        0x0800
// Num lock active.
#define BSP_MODKEY_NUM_LK    0x1000
// Caps lock active.
#define BSP_MODKEY_CAPS_LK   0x2000
// Scroll lock active.
#define BSP_MODKEY_SCROLL_LK 0x8000
// Any control key pressed.
#define BSP_MODKEY_CTRL      (BSP_MODKEY_L_CTRL | BSP_MODKEY_R_CTRL)
// Any shift key pressed.
#define BSP_MODKEY_SHIFT     (BSP_MODKEY_L_SHIFT | BSP_MODKEY_R_SHIFT)
// Any alt kay pressed.
#define BSP_MODKEY_ALT       (BSP_MODKEY_L_ALT | BSP_MODKEY_R_ALT)
