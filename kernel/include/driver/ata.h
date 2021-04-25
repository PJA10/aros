#ifndef _DRIVER_SERIAL_H
#define _DRIVER_SERIAL_H

#define REG_DATA 0
#define REG_SEC_COUNT 2
#define REG_LBA_LO 3
#define REG_LBA_MID 4
#define REG_LBA_HI 5
#define REG_DEVSEL 6
#define REG_STATUS 7
#define REG_COMMAND 7


#define IDENTIFY_COMMAND 0xEC

#define ERR 1
#define DRQ 8
#define BSY 0x80


#endif
