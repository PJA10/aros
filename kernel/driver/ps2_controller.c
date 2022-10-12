#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <common.h>

#include <driver/ps2_controller.h>

#include <sys/io.h>

#define DATA_PORT (0x60)
#define STATUS_REG (0x64)
#define COMMAND_REG (0x64)

#define READ_CONFIG_BYTE_CMD (0x20)
#define WRITE_CONFIG_BYTE_CMD (0x60)
#define DISABLE_SECOND_PS2_CMD (0xA7)
#define ENABLE_SECOND_PS2_CMD (0xA8)
#define TEST_SECOND_PORT_CMD (0xA9)
#define TEST_PS2_CONTROLLER_CMD (0xAA)
#define TEST_FIRST_PORT_CMD (0xAB)
#define DISABLE_FIRST_PS2_CMD (0xAD)
#define ENABLE_FIRST_PS2_CMD (0xAE)

#define OUTPUT_BUFFER_FULL_FLAG (0x1)
#define INPUT_BUFFER_FULL_FLAG (0x2)

#define FIRST_PORT_INTERRUPT_ENABLED (0x1)
#define SECOND_PORT_INTERRUPT_ENABLED (0x2)
#define FIRST_PORT_TRANSLATION_ENABLED (0x40)

#define PS2_TEST_PASS (0x55)
#define PORT_TEST_PASS (0)

#define RESEET_DEVICE_CMD (0xFF)

#define RESET_SUCCESS (0xFA)
#define SELF_TEST_PASSED (0xAA)

#define WAIT_FOR_READY_TIMEOUT (100)

bool ps2_keyboard__is_init = false;

static void send_command(uint8_t command) {
    outb(COMMAND_REG, command);
}

static uint8_t get_status() {
    return inb(STATUS_REG);
}

return_code_t ps2_controller__read_data(uint8_t *out_data) {
    return_code_t return_code = RETURN_CODE_UNINIALIZED;
    uint8_t counter = 0;

    while (!(get_status() & OUTPUT_BUFFER_FULL_FLAG)) {
        if (counter > WAIT_FOR_READY_TIMEOUT) {
            printf("ps2 contoller ps2_controller__read_data failed, output buffer isn't full - status: 0x%x\n", get_status());
            return_code = RETURN_CODE_PS2_CONTROLLER__READ_DATA_TIMEOUT;
            goto cleanup;
        }
        counter++;
    }
    printf("reading\n");
    *out_data = inb(DATA_PORT);
    printf("ps2_controller read: 0x%x\n", *out_data);
    return_code = RETURN_CODE_SUCCESS;

cleanup:
    return return_code;
}

return_code_t ps2_controller__send_data(uint8_t value) {
    return_code_t return_code = RETURN_CODE_UNINIALIZED;
    uint8_t counter = 0;

    while (get_status() & INPUT_BUFFER_FULL_FLAG) {
        if (counter > WAIT_FOR_READY_TIMEOUT) {
            printf("ps2 contoller send_data failed, input buffer isn't full - status: 0x%x\n", get_status());
            return_code = RETURN_CODE_PS2_CONTROLLER__SEND_DATA_TIMEOUT;
            goto cleanup;
        }
        counter++;
    }
    outb(DATA_PORT, value);
    return_code = RETURN_CODE_SUCCESS;

cleanup:
    return return_code;
}

return_code_t ps2_controller__init() {
    return_code_t return_code = RETURN_CODE_UNINIALIZED;
    uint8_t data = 0;

    // Disable Devices
    send_command(DISABLE_SECOND_PS2_CMD);
    send_command(DISABLE_FIRST_PS2_CMD);

    // Flush The Output Buffer
    inb(DATA_PORT);
    if (get_status() & OUTPUT_BUFFER_FULL_FLAG)
    {
        return_code = RETURN_CODE_PS2_CONTROLLER__INIT_COULDNT_FLUSH_OUTPUT_BUFFER;
        goto cleanup;
    }
    
    // Set the Controller Configuration Byte
    send_command(READ_CONFIG_BYTE_CMD);
    uint8_t old_config = 0;
    return_code = ps2_controller__read_data(&old_config);
    CHECK_SUCCESS();

    uint8_t new_config = (old_config | FIRST_PORT_INTERRUPT_ENABLED | SECOND_PORT_INTERRUPT_ENABLED) & ~FIRST_PORT_TRANSLATION_ENABLED;
    printf("new_config: 0x%x\n", new_config);
    send_command(WRITE_CONFIG_BYTE_CMD);
    return_code = ps2_controller__send_data(new_config);
    CHECK_SUCCESS();

    // Perform Controller Self Test
    send_command(TEST_PS2_CONTROLLER_CMD);
    return_code = ps2_controller__read_data(&data);
    CHECK_SUCCESS();
    if (data != PS2_TEST_PASS) {
        return_code = RETURN_CODE_PS2_CONTROLLER__INIT_CONTROLLER_RESET_FAILED;
        goto cleanup;
    }

    // TODO: Determine If There Are 2 Channels
    bool is_two_channel = false;

    // Perform Interface Tests
    send_command(TEST_FIRST_PORT_CMD);
    ps2_controller__read_data(&data);
    CHECK_SUCCESS();
    if (data != PORT_TEST_PASS) {
        return_code = RETURN_CODE_PS2_CONTROLLER__INIT_PORT_TEST_FAILED;
        goto cleanup;
    }
    if (is_two_channel) {
        send_command(TEST_SECOND_PORT_CMD);
        ps2_controller__read_data(&data);
        CHECK_SUCCESS();
        if (data != PORT_TEST_PASS) {
            return_code = RETURN_CODE_PS2_CONTROLLER__INIT_PORT_TEST_FAILED;
            goto cleanup;
        }
    }

    // Enable Devices
    send_command(ENABLE_FIRST_PS2_CMD);
    if (is_two_channel) {
        send_command(ENABLE_SECOND_PS2_CMD);
    }

    // Reset Devices
    return_code = ps2_controller__send_data(RESEET_DEVICE_CMD);
    CHECK_SUCCESS();
    return_code = ps2_controller__read_data(&data);
    CHECK_SUCCESS();
    if (data != RESET_SUCCESS) {
        return_code = RETURN_CODE_PS2_CONTROLLER__INIT_DEVICE_RESET_FAILED;
        goto cleanup;
    } 
    return_code = ps2_controller__read_data(&data);
    CHECK_SUCCESS();
    if (data != SELF_TEST_PASSED) {
        return_code = RETURN_CODE_PS2_CONTROLLER__INIT_DEVICE_RESET_FAILED;
        goto cleanup;
    }
    // TODO: reset second port device

    return_code = RETURN_CODE_SUCCESS;

cleanup:
    return return_code;
}
