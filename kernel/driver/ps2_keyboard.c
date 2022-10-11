#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <common.h>

#include <driver/ps2_keyboard.h>
#include <driver/ps2_controller.h>

#include <sys/io.h>

#define KEYBOARD_ACK (0xFA)

static return_code_t check_ack() {
    return_code_t return_code = RETURN_CODE_UNINIALIZED;
    uint8_t data = 0;

    return_code = ps2_controller__read_data(&data);
    CHECK_SUCCESS();
    if (data != KEYBOARD_ACK) {
        return_code = RETURN_CODE_FAILURE;
        goto cleanup;
    }

cleanup:
    return return_code;
}

static return_code_t get_scan_code(uint8_t *out_scan_code) {
    return_code_t return_code = RETURN_CODE_UNINIALIZED;
    uint8_t scan_code = 0;

    return_code = ps2_controller__send_data(0xF0);
    CHECK_SUCCESS();
    return_code = check_ack();
    CHECK_SUCCESS();

    return_code = ps2_controller__send_data(0);
    CHECK_SUCCESS();
    return_code = check_ack();
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
    
    uint8_t scan_code = 0;
    return_code = get_scan_code(&scan_code);
    CHECK_SUCCESS();
    printf("scan_code: %x\n", scan_code);
    
    return_code = RETURN_CODE_SUCCESS;

cleanup:
    return 0;
}
