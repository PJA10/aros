#ifndef _DRIVER_ATA_H
#define _DRIVER_ATA_H

#include <common.h>
#include <stdint.h>

return_code_t ata_init(int slavebit);
return_code_t ata_read(int sector_num, int slavebit, void *buffer, int sector_count);
return_code_t ata_write(int sector_num, int slavebit, void *buffer, int sector_count);

#endif
