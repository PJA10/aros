#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

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

#define TIMEOUT (100)

bool ps2_controller__is_init = false;

static void send_command(uint8_t command) {
    outb(COMMAND_REG, command);
}

static uint8_t get_status() {
    return inb(STATUS_REG);
}

uint8_t ps2_controller__read_data() {
    uint8_t counter = 0;
    while (!(get_status() & OUTPUT_BUFFER_FULL_FLAG)) {
        if (counter > TIMEOUT) {
            printf("ps2 contoller ps2_controller__read_data failed, output buffer isn't full - status: 0x%x\n", get_status());
            return -1;
        }
        counter++;
    }
    return inb(DATA_PORT);
}

int ps2_controller__send_data(uint8_t value) {
    uint8_t counter = 0;
    while (get_status() & INPUT_BUFFER_FULL_FLAG) {
        if (counter > TIMEOUT) {
            printf("ps2 contoller send_data failed, input buffer isn't full - status: 0x%x\n", get_status());
            return -1;
        }
        counter++;
    }
    outb(DATA_PORT, value);
    return 0;
}

int ps2_controller__init() {
    // Disable Devices
    send_command(DISABLE_SECOND_PS2_CMD);
    send_command(DISABLE_FIRST_PS2_CMD);

    // Flush The Output Buffer
    inb(DATA_PORT);
    if (get_status() & OUTPUT_BUFFER_FULL_FLAG)
    {
        printf("ps2_controller__init failed, couldn't flush output buffer\n");
        return -1;
    }
    
    // Set the Controller Configuration Byte
    send_command(READ_CONFIG_BYTE_CMD);
    uint8_t old_config = ps2_controller__read_data();
	printf("old_config: 0x%x\n", old_config);
    uint8_t new_config = (old_config | FIRST_PORT_INTERRUPT_ENABLED | SECOND_PORT_INTERRUPT_ENABLED | FIRST_PORT_TRANSLATION_ENABLED);
    send_command(WRITE_CONFIG_BYTE_CMD);
    outb(DATA_PORT, new_config);

    // Perform Controller Self Test
    send_command(TEST_PS2_CONTROLLER_CMD);
    if (ps2_controller__read_data() != PS2_TEST_PASS) {
        printf("ps2_controller__init failed, controller test failed.\n");
        return -1;
    }

    // TODO: Determine If There Are 2 Channels
    bool is_two_channel = true;

    // Perform Interface Tests
    send_command(TEST_FIRST_PORT_CMD);
    if (ps2_controller__read_data() != PORT_TEST_PASS) {
        printf("ps2_controller__init failed, first port test failed.\n");
        return -1;
    }
    if (is_two_channel) {
        send_command(TEST_SECOND_PORT_CMD);
        if (ps2_controller__read_data() != PORT_TEST_PASS) {
            printf("ps2_controller__init failed, first port test failed.\n");
            return -1;
        }
    }

    // Enable Devices
    send_command(ENABLE_FIRST_PS2_CMD);
    if (is_two_channel) {
        send_command(ENABLE_SECOND_PS2_CMD);
    }

    // Reset Devices
    ps2_controller__send_data(RESEET_DEVICE_CMD);
    uint8_t reset_res = ps2_controller__read_data();
    if (reset_res != RESET_SUCCESS) {
        printf("ps2_controller__init failed, first port reset failed - got 0x%x.\n", reset_res);
        return -1;
    }
    reset_res = ps2_controller__read_data();
    if (reset_res != 0xAA) {
        printf("ps2_controller__init failed, first port reset failed - got 0x%x.\n", reset_res);
        return -1;
    }
    // TODO: reset second port device

    ps2_controller__is_init = true;
    return 0;
}