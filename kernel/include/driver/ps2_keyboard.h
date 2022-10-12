#ifndef _DRIVER_PS2_KEYBOARD_H
#define _DRIVER_PS2_KEYBOARD_H

enum keys_e {
    F1_KEY,
    F2_KEY,
    F3_KEY,
    F4_KEY,
    F5_KEY,
    F6_KEY,
    F7_KEY,
    F8_KEY,
    F9_KEY,
    F10_KEY,
    F11_KEY,
    F12_KEY,

    LEFT_ALT_KEY,
    LEFT_SHIFT_KEY,
    LEFT_CTRL_KEY,

    CAPSLOCK_KEY,
    RIGHT_SHIFT_KEY,
    ENTER_KEY,
    ESCAPE_KEY,
    NUMBERLOCK_KEY,
};

int ps2_keyboard__init();
void ps2_keyboard__irq_handler();

#endif
