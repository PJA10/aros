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
    unsigned char scan_code = 0;
    ps2_controller__read_data(&scan_code);
    printf("IRQ1 handler - keybord interrupt - key preesed: 0x%x\n", scan_code);
}
