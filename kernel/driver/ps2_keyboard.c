#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <common.h>

#include <driver/ps2_keyboard.h>
#include <driver/ps2_controller.h>

#include <sys/io.h>

#define KEYBOARD_ACK (0xFA)

#define SCAN_CODE_SET_2 (2)
#define SCAN_CODE_SET_2_ANOTHER_ID (41)

#define GET_SET_CURRENT_SCAN_CODE_SET_COMMAND (0xF0)
#define ENABLE_SCANNING_COMMNAD (0xF4)
#define DISABLE_SCANNING_COMMNAD (0xF5)

#define SHIFT_STATE_INDEX (0)
#define CTRL_STATE_INDEX (1)
#define ALT_STATE_INDEX (2)

typedef enum keyboard_driver_state_e {
    DEFAULT_STATE, BREAK_STATE,
} keyboard_driver_state_t;

static uint8_t us_qwerty_set_2[] = {
    [0x01] F9_KEY, [0x03] F5_KEY, [0x04] F3_KEY, [0x05] F1_KEY, [0x06] F2_KEY, [0x07] F12_KEY, [0x09] F10_KEY, [0x0A] F8_KEY, [0x0B] F6_KEY, [0x0C] F4_KEY, [0x0D] '\t', [0x0E] '`',
    [0x11] LEFT_ALT_KEY, [0x12] LEFT_SHIFT_KEY, [0x14] LEFT_CTRL_KEY, [0x15] 'q', [0x16] '1', [0x1A] 'z', [0x1B] 's', [0x1C] 'a', [0x1D] 'w', [0x1E] '2', [0x21] 'c', [0x22] 'x', [0x23] 'd', [0x24] 'e', [0x25] '4', [0x26] '3', [0x29] ' ',
    [0x2A] 'v', [0x2B] 'f', [0x2C] 't', [0x2D] 'r', [0x2e] '5', [0x31] 'n', [0x32] 'b', [0x33] 'h', [0x34] 'g', [0x35] 'y', [0x36] '6', [0x3A] 'm', [0x3B] 'j', [0x3C] 'u', [0x3D] '7', [0x3E] '8', [0x41] ',', [0x42] 'k', [0x43] 'i', [0x44] 'o', [0x45] '0', [0x46] '9', [0x49] '.',
    [0x4A] '/', [0x4B] 'l', [0x4C] ';', [0x4D] 'p', [0x4E] '-', [0x52] '\'', [0x54] '[', [0x55] '=', [0x58] CAPSLOCK_KEY, [0x59] RIGHT_SHIFT_KEY, [0x5A] '\n', [0x5B] ']', [0x5D] '\\', [0x66] '\b', [0x76] ESCAPE_KEY, [0x77] NUMBERLOCK_KEY, [0x78] F11_KEY, [0x83] F7_KEY,
};

static uint8_t us_qwerty_set_2_shift[] = {
    [0x01] F9_KEY, [0x03] F5_KEY, [0x04] F3_KEY, [0x05] F1_KEY, [0x06] F2_KEY, [0x07] F12_KEY, [0x09] F10_KEY, [0x0A] F8_KEY, [0x0B] F6_KEY, [0x0C] F4_KEY, [0x0D] '\t', [0x0E] '~',
    [0x11] LEFT_ALT_KEY, [0x12] LEFT_SHIFT_KEY, [0x14] LEFT_CTRL_KEY, [0x15] 'Q', [0x16] '!', [0x1A] 'Z', [0x1B] 'S', [0x1C] 'A', [0x1D] 'W', [0x1E] '@', [0x21] 'C', [0x22] 'X', [0x23] 'D', [0x24] 'E', [0x25] '$', [0x26] '#', [0x29] ' ',
    [0x2A] 'V', [0x2B] 'F', [0x2C] 'T', [0x2D] 'R', [0x2e] '%', [0x31] 'N', [0x32] 'B', [0x33] 'H', [0x34] 'G', [0x35] 'Y', [0x36] '^', [0x3A] 'M', [0x3B] 'J', [0x3C] 'U', [0x3D] '&', [0x3E] '*', [0x41] '<', [0x42] 'K', [0x43] 'I', [0x44] 'O', [0x45] ')', [0x46] '(', [0x49] '>',
    [0x4A] '?', [0x4B] 'L', [0x4C] ':', [0x4D] 'P', [0x4E] '_', [0x52] '"', [0x54] '{', [0x55] '+', [0x58] CAPSLOCK_KEY, [0x59] RIGHT_SHIFT_KEY, [0x5A] '\n', [0x5B] '}', [0x5D] '|', [0x66] '\b', [0x76] ESCAPE_KEY, [0x77] NUMBERLOCK_KEY, [0x78] F11_KEY, [0x83] F7_KEY,
};

static bool keys_state[] = {
    [SHIFT_STATE_INDEX] = 0,
    [CTRL_STATE_INDEX] = 0, 
    [ALT_STATE_INDEX] = 0,
};

static return_code_t check_ack() {
    return_code_t return_code = RETURN_CODE_UNINIALIZED;
    uint8_t data = 0;

    return_code = ps2_controller__read_data(&data);
    CHECK_SUCCESS();
    if (data != KEYBOARD_ACK) {
        return_code = RETURN_CODE_PS2_KEYBOARD__NO_ACK_RECEIVED;
        goto cleanup;
    }

cleanup:
    return return_code;
}

static return_code_t send_to_keyboard(uint8_t data) {
    return_code_t return_code = RETURN_CODE_UNINIALIZED;
    
    return_code = ps2_controller__send_data(data);
    CHECK_SUCCESS();
    return_code = check_ack();
    CHECK_SUCCESS();

    return_code = RETURN_CODE_SUCCESS;

cleanup:
    return return_code;
}

static return_code_t set_scan_code(uint8_t scan_code_set) {
    return_code_t return_code = RETURN_CODE_UNINIALIZED;

    return_code = send_to_keyboard(GET_SET_CURRENT_SCAN_CODE_SET_COMMAND);
    CHECK_SUCCESS();

    return_code = send_to_keyboard(scan_code_set);
    CHECK_SUCCESS();

    return_code = RETURN_CODE_SUCCESS;

cleanup:
    return return_code;
}

static return_code_t get_scan_code(uint8_t *out_scan_code) {
    return_code_t return_code = RETURN_CODE_UNINIALIZED;
    uint8_t scan_code = 0;

    return_code = send_to_keyboard(GET_SET_CURRENT_SCAN_CODE_SET_COMMAND);
    CHECK_SUCCESS();

    return_code = send_to_keyboard(0);
    CHECK_SUCCESS();

    return_code = ps2_controller__read_data(&scan_code);
    CHECK_SUCCESS();

    *out_scan_code = scan_code;
    return_code = RETURN_CODE_SUCCESS;

cleanup:
    return return_code;
}

return_code_t ps2_keyboard__init() {
    return_code_t return_code = RETURN_CODE_UNINIALIZED;
    
    return_code = send_to_keyboard(DISABLE_SCANNING_COMMNAD);
    CHECK_SUCCESS();

    return_code = set_scan_code(SCAN_CODE_SET_2);
    CHECK_SUCCESS();

    uint8_t scan_code = 0;
    return_code = get_scan_code(&scan_code);
    CHECK_SUCCESS();
    if (scan_code != SCAN_CODE_SET_2 && scan_code != SCAN_CODE_SET_2_ANOTHER_ID) {
        return_code = RETURN_CODE_PS2_KEYBOARD__INIT_BAD_SCAN_CODE_SET;
        goto cleanup;
    }

    return_code = send_to_keyboard(ENABLE_SCANNING_COMMNAD);
    CHECK_SUCCESS();
    
    ps2_keyboard__is_init = true;
    return_code = RETURN_CODE_SUCCESS;

cleanup:
    return return_code;
}

void ps2_keyboard__irq_handler() {
    return_code_t return_code = RETURN_CODE_UNINIALIZED;
    static keyboard_driver_state_t state = DEFAULT_STATE;
    unsigned char scan_code = 0;

    return_code = ps2_controller__read_data(&scan_code);
    CHECK_SUCCESS();

    uint8_t key_code = us_qwerty_set_2[scan_code];
    
    switch (state) {
        case DEFAULT_STATE:
                if (scan_code == 0xF0) {
                    state = BREAK_STATE;
                }
                else if (scan_code < sizeof(us_qwerty_set_2) / sizeof(us_qwerty_set_2[0])) {
                    if (key_code == LEFT_SHIFT_KEY || key_code == RIGHT_SHIFT_KEY) {
                        keys_state[SHIFT_STATE_INDEX] = true;
                    }
                    else if (key_code == LEFT_CTRL_KEY) {
                        keys_state[CTRL_STATE_INDEX] = true;
                    }
                    else if (keys_state[SHIFT_STATE_INDEX]) {
                        putchar(us_qwerty_set_2_shift[scan_code]);
                    }
                    else {
                        putchar(us_qwerty_set_2[scan_code]);
                    }
                }
                else {
                    printf("IRQ1 handler - keybord interrupt - scan code: 0x%x\n", scan_code);
                }
            break;

        case BREAK_STATE:
            if (key_code == LEFT_SHIFT_KEY || key_code == RIGHT_SHIFT_KEY) {
                keys_state[SHIFT_STATE_INDEX] = false;
            }
            else if (key_code == LEFT_CTRL_KEY) {
                keys_state[CTRL_STATE_INDEX] = false;
            }
            state = DEFAULT_STATE;
            break;

        default:
            return_code = RETURN_CODE_FAILURE;
            goto cleanup;
    }
    return_code = RETURN_CODE_SUCCESS;

cleanup:
    if (return_code != RETURN_CODE_SUCCESS) {
        printf("ps2 keyboard irq handler got error: %d\n", return_code);
        abort();
    }
}
