#ifndef _DRIVER_ATA_H
#define _DRIVER_ATA_H

#include <stdint.h>

int ata_init(int slavebit);
void ata_read(int sector_num, int slavebit, void *buffer, int sector_count);
void ata_write(int sector_num, int slavebit, void *buffer, int sector_count);

#endif
