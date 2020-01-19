#include <kernel/drivers/storage/ATA_PIO/ATA_PIO.h>
#include <klibc/stdlib.h>
#include <klibc/stdio.h>
#include <kernel/asm.h>
#include <kernel/drivers/PIC/PIC.h>
#include <kernel/IDT/IDT.h>

struct ATA_INTERFACE* newATA(uint8_t master, uint16_t portBase) {
	struct ATA_INTERFACE* ret = (struct ATA_INTERFACE*)jmalloc(sizeof(struct ATA_INTERFACE));

	ret->master = master;
	ret->dataPort = portBase;
	ret->errorPort = portBase + 0x1;
	ret->sectorCountPort = portBase + 0x2;
	ret->lbaLowPort = portBase + 0x3;
	ret->lbaMidPort = portBase + 0x4;
	ret->lbaHiPort = portBase + 0x5;
	ret->devicePort = portBase + 0x6;
	ret->commandPort = portBase + 0x7;
	ret->controlPort = portBase + 0x206;

	ISR_ignoreUP(0x2E);
	ISR_ignoreUP(0x2F);

	return ret;
}

uint8_t ATA_identify(struct ATA_INTERFACE* iface, uint16_t* retdata) {
	outb(iface->devicePort, iface->master ? 0xA0 : 0xB0);
	outb(iface->controlPort, 0);

	outb(iface->devicePort, 0xA0);
	uint8_t status = inb(iface->commandPort);
	if(status == 0xFF) return 1;

	outb(iface->devicePort, iface->master ? 0xA0 : 0xB0);
	outb(iface->sectorCountPort, 0);
	outb(iface->lbaLowPort, 0);
	outb(iface->lbaMidPort, 0);
	outb(iface->lbaHiPort, 0);
	outb(iface->commandPort, 0xEC); // Identify command

	status = inb(iface->commandPort);
	if(!status) return 1;

	while(((status & 0x80) == 0x80) && ((status & 0x01) != 0x01)) {
		status = inb(iface->commandPort);
	}

	if(status & 0x01) return 1;

	for(int i=0; i<256; i++) retdata[i] = inw(iface->dataPort);
	return 0;
}

uint8_t* ATA_read28(struct ATA_INTERFACE* iface, uint32_t sector) {
	if(sector > 0x0FFFFFFF) return 0;

	outb(iface->devicePort, (iface->master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));

	// Wait for drive select to finish.
	uint8_t status;
	for(int i=0; i<5; i++) status = inb(iface->commandPort);
	if(status == 0xFF) return 0;

	outb(iface->errorPort, 0);
	outb(iface->sectorCountPort, 1);
	outb(iface->lbaLowPort, sector & 0x000000FF);
	outb(iface->lbaMidPort, (sector & 0x0000FF00) >> 8);
	outb(iface->lbaHiPort, (sector & 0x00FF0000) >> 16);
	outb(iface->commandPort, 0x20);	// Read command.

	status = inb(iface->commandPort);
	while((status & 0x80) && !(status & 0x01)) status = inb(iface->commandPort);

	uint8_t* ret = (uint8_t*)jmalloc(BYTES_PER_SECTOR);
	for(int i=0; i<BYTES_PER_SECTOR; i+=2) {
		uint16_t data = inw(iface->dataPort);
		ret[i] = data & 0xFF;
		ret[i+1] = (data >> 8) & 0xFF;
	}

	return ret;
}

uint8_t ATA_write28(struct ATA_INTERFACE* iface, uint32_t sector, uint8_t* contents) {
	if(sector > 0x0FFFFFFF) return 1;
	cli();

	// WRITE
	outb(iface->devicePort, (iface->master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));

	// Check that drive select worked.
	uint8_t status;
	for(int i=0; i<5; i++) status = inb(iface->commandPort);
	if(status == 0xFF) return 1;

	outb(iface->errorPort, 0);
	outb(iface->sectorCountPort, 1);
	outb(iface->lbaLowPort, sector & 0x000000FF);
	outb(iface->lbaMidPort, (sector & 0x0000FF00) >> 8);
	outb(iface->lbaHiPort, (sector & 0x00FF0000) >> 16);
	outb(iface->commandPort, 0x30);	// Write command.

	// Wait until BSY=0 and DRQ=1.
	while((status & 0x80) || !(status & 0x08)) status = inb(iface->commandPort);

	// Check ERR and DF are not set.
	if(status & (0x01 || 0x20)) return 2;

	// Start sending the data.
	for(int i=0; i<BYTES_PER_SECTOR; i+=2) {
		uint16_t data = contents[i];
		data |= ((uint16_t)contents[i+1]) << 8;
		outw(iface->dataPort, data);
	}

	// FLUSH
	// No need to drive select again.
	outb(iface->commandPort, 0xE7);	// Flush command.

	for(int i=0; i<5; i++) status = inb(iface->commandPort);
	if(!status) return 3;

	while((status & 0x80) && !(status & 0x01)) {
		status = inb(iface->commandPort);
	}

	pic_refresh();
	return 0;
}

uint8_t ATA_clear28(struct ATA_INTERFACE* iface, uint32_t sector) {
	uint8_t emptysector[512] = {0};
	return ATA_write28(iface, sector, emptysector);
}
