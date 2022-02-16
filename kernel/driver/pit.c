#include <driver/pit.h>
#include <arch-i386/pic.h>

#include <sys/io.h>

#include <stdio.h>

#define CHANNEL_0_DATA_PORT 0x40
#define CHANNEL_1_DATA_PORT 0x41
#define CHANNEL_2_DATA_PORT 0x42
#define MODE_COMMAND_REG 0x43

#define LOBYTE_ONLY_ACCESS_MODE 0x1
#define HIBYTE_ONLY_ACCESS_MODE 0x2
#define LOBYTE_HIBYTE_ACCESS_MODE 0x3

#define INTERRUPT_ON_TERMINAL_COUNT_MODE 0x0
#define HARDWARE_RE_TRIGGERABLE_ONE_SHOT_MODE 0x1 // only for channel 2
#define RATE_GENERATOR_MODE 0x2
#define SQUARE_WAVE_GENERATOR_MODE 0x3
#define SOFTWARE_TRIGGERED_STROBE_MODE 0x4
#define HARDWARE_TRIGGERED_STROBE_MODE 0x5
// #define RATE_GENERATOR_MODE 0x6 - same as 010b
// #define SQUARE_WAVE_GENERATOR_MODE 0x7 - ssame as 011b

#define BASE_FREQ 1193182 // Hz
#define TARGET_FREQ 1000 // Hz

static uint32_t ticks;

void set_mode(uint8_t channel, uint8_t access_mode, uint8_t operatint_mode) {
    uint8_t command = (uint8_t)((channel << 6) | access_mode << 4 | operatint_mode << 1);
    outb(MODE_COMMAND_REG, command);
}

void pit_init() {
    set_mode(0, LOBYTE_HIBYTE_ACCESS_MODE, RATE_GENERATOR_MODE);
    int divider = BASE_FREQ /TARGET_FREQ;
    outb(CHANNEL_0_DATA_PORT, (uint8_t)divider&0xff);
    outb(CHANNEL_0_DATA_PORT, (uint8_t)((divider&0xff00)>>0x8));
    IRQ_clear_mask((unsigned char) 0); // unmask IRQ0 - timer timer interrupt
    ticks_to_nanoseconds = (uint64_t) 1000000000 * divider / BASE_FREQ ; // == 1000000000 * (1/(BASE_FREQ/divider), because Hz^-1 = 1000000000 ns
}

void pit_interrupt() {
    ticks++;
    //printf("ticks: %u, nanoseconds: %q\n", ticks, get_nanoseconds_since_boot());
}

inline uint64_t get_nanoseconds_since_boot() {
    return ticks * ticks_to_nanoseconds;
}