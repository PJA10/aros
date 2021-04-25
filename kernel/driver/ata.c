#include <sys/io.h>
#include <stdio.h>
#include <driver/ata.h>


int ata_init(int slavebit) {
	int io_base = 0x1F0;
	int ctl_base = 0x3F6;
	int status = inb(io_base + REG_STATUS);
	if (status == 0xFF) {
		// Floating Bus
		printf("error in ata init - Floating Bus\n");
		return -1;
	}
	outb(io_base + REG_DEVSEL, 0xA0 | slavebit<<4); // select master/slave drive
	outb(io_base + REG_SEC_COUNT, 0);
	outb(io_base + REG_LBA_LO, 0);
	outb(io_base + REG_LBA_MID, 0);
	outb(io_base + REG_LBA_HI, 0);

	outb(io_base + REG_COMMAND, IDENTIFY_COMMAND);
	
	status = inb(io_base + REG_STATUS);
	if (status == 0) {
		// drive does not exist
		printf("error in ata init - %d driver does not exist\n", slavebit);
		return -1;
	}
	while ((status = inb(io_base + REG_STATUS)) & BSY) {
		// debug
		// printf("ata init waiting for BSY status=%x\n", status);
	}

	if (inb(io_base + REG_LBA_MID) != 0 || inb(io_base + REG_LBA_HI) != 0) {
		// drive is not ATA
		printf("error in ata init - %d driver is not ATA\n", slavebit);
		return -1;
	}

	// TODO: add support to "Command Aborted"
	while (!(status = inb(io_base + REG_STATUS)) & DRQ | !status & ERR) { 
		// debug
		// printf("ata init waiting for DRQ or ERR status=%x\n", status);
	}

	if (status & ERR) {
		// error
		printf("error in ata init - %d driver\n", slavebit);
		return -1;
	}

	uint16_t identify_data[256];
	for (int i = 0; i < sizeof(identify_data)/sizeof(identify_data[0]); i++) {
		identify_data[i] = inw(io_base + REG_DATA);
		//printf("%d: 0x%x\n", i, (int) identify_data[i]);
	}
	int support_LBA48 = identify_data[83] & 0x400;
	int supported_UDMA_modes = identify_data[88] & 0xFF;
	int active_UDMA_mode = identify_data[88] & 0xFF00;
	int conductor_80_cable = identify_data[93] & 0x800;
	uint32_t num_LBA28_sectors = (identify_data[61] << 16) | identify_data[60];
	uint64_t num_LBA48_sectors = (identify_data[103] << 48) | (identify_data[102] << 32) | (identify_data[101] << 16) | identify_data[100];
	
	// printf("IDENTIFY info:\n");
	// printf("support_LBA48: %d\n", support_LBA48);
	// printf("supported_UDMA_modes: 0x%x, active_UDMA_mode: 0x%x\n", supported_UDMA_modes, active_UDMA_mode >> 8);
	// printf("conductor_80_cable: %d\n", conductor_80_cable);
	// printf("num_LBA28_sectors: 0x%x\n", num_LBA28_sectors);
	// printf("num_LBA28_sectors: 0x%x\n", num_LBA28_sectors);
	// printf("num_LBA48_sectors: 0x%x << 32 | 0x%x\n", (int) (num_LBA48_sectors >> (32)), (int)(num_LBA48_sectors << 32 >> 32));

	return 0;
}