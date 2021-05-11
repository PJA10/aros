#include <driver/ata.h>

#include <sys/io.h>

#include <kernel/heap.h>

#include <stdio.h>
#include <string.h>

#define REG_DATA 0
#define REG_ALT_STATUS 0
#define REG_ERR 1
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
#define RDY 0x40
#define BSY 0x80

static void ata_read_write(int LBA, int slavebit, void *buffer, int sector_count, int write);

static int io_base;
static int ctl_base;

int ata_init(int slavebit) {
	io_base = 0x1F0;
	ctl_base = 0x3F6;
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
	while (!((status = inb(io_base + REG_STATUS)) & DRQ) && !(status & ERR)) { 
		// debug
		printf("ata init waiting for DRQ or ERR status=0x%x\n", status);
	}

	if (status & ERR) {
		// error
		printf("error in ata init - %d driver\n", slavebit);
		return -1;
	}

	uint16_t identify_data[256];
	for (int i = 0; i < 256; i++) {
		identify_data[i] = inw(io_base + REG_DATA);
	}
	int support_LBA48 = identify_data[83] & 0x400;
	int supported_UDMA_modes = identify_data[88] & 0xFF;
	int active_UDMA_mode = identify_data[88] & 0xFF00;
	int conductor_80_cable = identify_data[93] & 0x800;
	uint32_t num_LBA28_sectors = (identify_data[61] << 16) | identify_data[60];
	uint64_t num_LBA48_sectors = ((uint64_t)identify_data[103] << 48) | ((uint64_t)identify_data[102] << 32) | (identify_data[101] << 16) | identify_data[100];
	
	// debug
	// printf("IDENTIFY info:\n");
	// printf("support_LBA48: %d\n", support_LBA48);
	// printf("supported_UDMA_modes: 0x%x, active_UDMA_mode: 0x%x\n", supported_UDMA_modes, active_UDMA_mode >> 8);
	// printf("conductor_80_cable: %d\n", conductor_80_cable);
	// printf("num_LBA28_sectors: 0x%x\n", num_LBA28_sectors);
	// printf("num_LBA48_sectors: 0x%x << 32 | 0x%x\n", (int) (num_LBA48_sectors >> (32)), (int)(num_LBA48_sectors << 32 >> 32));
	return 0;
}

void ata_read(int sector_num, int slavebit, void *buffer, int sector_count) {
	ata_read_write(sector_num, slavebit, buffer, sector_count, 0);
}

void ata_write(int sector_num, int slavebit, void *buffer, int sector_count) {
	ata_read_write(sector_num, slavebit, buffer, sector_count, 1);
}

static void ata_read_write(int LBA, int slavebit, void *buffer, int sector_count, int write) {
	if (sector_count > 256) {
		return;
	} else if (sector_count == 256) {
		sector_count = 0; // 0 represnts 256 sectors
	}
	while (inb(io_base + REG_STATUS) & BSY)
	; // Wait for BSY to be zero.
	
	outb(io_base + REG_DEVSEL, 0xE0 | (slavebit << 4) | ((LBA >> 24) & 0x0F));
	outb(io_base + REG_SEC_COUNT, (unsigned char) sector_count);
	outb(io_base + REG_LBA_LO, (unsigned char) LBA);
	outb(io_base + REG_LBA_MID, (unsigned char) (LBA >> 8));
	outb(io_base + REG_LBA_HI, (unsigned char) (LBA >> 16));
	int cmd = (sector_count == 1) ? READ_COMMAND :  READ_MULTIPLE_COMMAND;
	if (write) {
		cmd = (sector_count == 1) ? WRITE_COMMAND :  WRITE_MULTIPLE_COMMAND;
	}
	//printf("cmd: 0x%x, LBA: %d, buffer: 0x%x, sector_count: %d\n", cmd, LBA, buffer, sector_count);
	outb(io_base + REG_COMMAND, cmd);

	if (sector_count == 0) {
		sector_count = 256;
	}
	int status;
	uint16_t *b = buffer;
	for (int j = 0; j < sector_count; j++) {
		for (int i = 0; i < 4; i++) {
			status = inb(ctl_base + REG_ALT_STATUS);
			//printf("ata read_write polling 400ms delay status=0x%x drive=%d j=%d LBA: %d\n", status, slavebit, j, LBA);
		}

		while ((((status = inb(io_base + REG_STATUS)) & BSY) || !(status & DRQ)) && !(status & ERR) && !(status & DF)) {
			// debug
			//printf("ata read_write waiting for DRQ or ERR status=0x%x drive=%d j=%d LBA: %d\n", status, slavebit, j, LBA);
		}

		if ((status & ERR) || (status & DF)) {
			printf("error in reading/writing %d sectors from LBA 0x%x in drive %d\n", sector_count, slavebit, slavebit);
			return;
		}
		for (int i = 0; i < 256; i++, b++) {
			if (write) {
				outw(io_base + REG_DATA, *b);
			} else {
				*b = inw(io_base + REG_DATA);
			}
		}
		if (write) {
			outb(io_base + REG_COMMAND, CACHE_FLUSH_COMMAND);
		}
	}
}