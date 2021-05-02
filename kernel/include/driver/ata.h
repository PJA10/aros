#ifndef _DRIVER_ATA_H
#define _DRIVER_ATA_H

#include <stdint.h>

#define REG_DATA 0
#define REG_SEC_COUNT 2
#define REG_LBA_LO 3
#define REG_LBA_MID 4
#define REG_LBA_HI 5
#define REG_DEVSEL 6
#define REG_STATUS 7
#define REG_COMMAND 7

#define IDENTIFY_COMMAND 0xEC
#define READ_COMMAND 0x20
#define READ_MULTIPLE_COMMAND 0xc4
#define WRITE_COMMAND 0x30
#define WRITE_MULTIPLE_COMMAND 0xc5
#define CACHE_FLUSH_COMMAND 0xe7

#define ERR 1
#define DRQ 8
#define DF 0x20
#define BSY 0x80

int ata_init(int slavebit);
void ata_read(int sector_num, int slavebit, void *buffer, int sector_count);
void ata_write(int sector_num, int slavebit, void *buffer, int sector_count);
void ata_read_write(int LBA, int slavebit, void *buffer, int sector_count, int write);

#endif
