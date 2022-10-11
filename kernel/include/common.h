#ifndef _COMMON_H
#define _COMMON_H

typedef enum return_code_e {
    RETURN_CODE_FAILURE = -1,
    RETURN_CODE_SUCCESS = 0,

    RETURN_CODE_UNINIALIZED,
    RETURN_CODE_PS2_CONTROLLER__SEND_DATA_TIMEOUT,
    RETURN_CODE_PS2_CONTROLLER__READ_DATA_TIMEOUT,
    RETURN_CODE_PS2_CONTROLLER__INIT_COULDNT_FLUSH_OUTPUT_BUFFER,
    RETURN_CODE_PS2_CONTROLLER__INIT_PORT_TEST_FAILED,
    RETURN_CODE_PS2_CONTROLLER__INIT_CONTROLLER_RESET_FAILED,
    RETURN_CODE_PS2_CONTROLLER__INIT_DEVICE_RESET_FAILED,
} return_code_t;

#define CHECK_SUCCESS()                         \
do {                                            \
    if (RETURN_CODE_SUCCESS != return_code) {   \
        goto cleanup;                           \
    }                                           \
} while(0)

#endif