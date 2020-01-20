#include <kernel/drivers/storage/ATA_PIO/ATA_PIO.h>
#include <klibc/stdlib.h>
#include <klibc/stdio.h>
#include <kernel/asm.h>
#include <kernel/drivers/PIC/PIC.h>
#include <kernel/IDT/IDT.h>

void ATA::init(uint8_t master, uint16_t portBase) {
	this->master = master;
	this->dataPort = portBase;

	errorPort = portBase+0x1;
	sectorCountPort = portBase+0x2;
	lbaLowPort = portBase+0x3;
	lbaMidPort = portBase+0x4;
	lbaHiPort = portBase+0x5;
	devicePort = portBase+0x6;
	commandPort = portBase+0x7;
	controlPort = portBase+0x206;

	ISR_ignoreUP(0x2E);
	ISR_ignoreUP(0x2F);
}

ATA::ATA() {}
ATA::ATA(uint8_t master, uint16_t portBase) { init(master, portBase); }
ATA::ATA(const ATA& other) { init(other.master, other.dataPort); }
ATA& ATA::operator=(const ATA& other) {
	init(other.master, other.dataPort);
	return *this;
}

uint8_t ATA::identify(uint16_t* retdata) {
	outb(devicePort, master ? 0xA0 : 0xB0);
	outb(controlPort, 0);

	outb(devicePort, 0xA0);
	uint8_t status = inb(commandPort);
	if(status == 0xFF) return 1;

	outb(devicePort, master ? 0xA0 : 0xB0);
	outb(sectorCountPort, 0);
	outb(lbaLowPort, 0);
	outb(lbaMidPort, 0);
	outb(lbaHiPort, 0);
	outb(commandPort, 0xEC); // Identify command

	status = inb(commandPort);
	if(!status) return 1;

	while(((status & 0x80) == 0x80) && ((status & 0x01) != 0x01)) {
		status = inb(commandPort);
	}

	if(status & 0x01) return 1;

	for(int i=0; i<256; i++) retdata[i] = inw(dataPort);
	return 0;
}

uint8_t* ATA::read28(uint32_t sector) {
	if(sector > 0x0FFFFFFF) return 0;

	outb(devicePort, (master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));

	// Wait for drive select to finish.
	uint8_t status;
	for(int i=0; i<5; i++) status = inb(commandPort);
	if(status == 0xFF) return 0;

	outb(errorPort, 0);
	outb(sectorCountPort, 1);
	outb(lbaLowPort, sector & 0x000000FF);
	outb(lbaMidPort, (sector & 0x0000FF00) >> 8);
	outb(lbaHiPort, (sector & 0x00FF0000) >> 16);
	outb(commandPort, 0x20);	// Read command.

	status = inb(commandPort);
	while((status & 0x80) && !(status & 0x01)) status = inb(commandPort);

	uint8_t* ret = (uint8_t*)jmalloc(BYTES_PER_SECTOR);
	for(int i=0; i<BYTES_PER_SECTOR; i+=2) {
		uint16_t data = inw(dataPort);
		ret[i] = data & 0xFF;
		ret[i+1] = (data >> 8) & 0xFF;
	}

	return ret;
}

uint8_t ATA::write28(uint32_t sector, uint8_t* contents) {
	if(sector > 0x0FFFFFFF) return 1;
	cli();

	// WRITE
	outb(devicePort, (master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));

	// Check that drive select worked.
	uint8_t status;
	for(int i=0; i<5; i++) status = inb(commandPort);
	if(status == 0xFF) return 1;

	outb(errorPort, 0);
	outb(sectorCountPort, 1);
	outb(lbaLowPort, sector & 0x000000FF);
	outb(lbaMidPort, (sector & 0x0000FF00) >> 8);
	outb(lbaHiPort, (sector & 0x00FF0000) >> 16);
	outb(commandPort, 0x30);	// Write command.

	// Wait until BSY=0 and DRQ=1.
	while((status & 0x80) || !(status & 0x08)) status = inb(commandPort);

	// Check ERR and DF are not set.
	if(status & (0x01 || 0x20)) return 2;

	// Start sending the data.
	for(int i=0; i<BYTES_PER_SECTOR; i+=2) {
		uint16_t data = contents[i];
		data |= ((uint16_t)contents[i+1]) << 8;
		outw(dataPort, data);
	}

	// FLUSH
	// No need to drive select again.
	outb(commandPort, 0xE7);	// Flush command.

	for(int i=0; i<5; i++) status = inb(commandPort);
	if(!status) return 3;

	while((status & 0x80) && !(status & 0x01)) {
		status = inb(commandPort);
	}

	pic_refresh();
	return 0;
}

static uint8_t emptysector[BYTES_PER_SECTOR] = {0};
uint8_t ATA::clear28(uint32_t sector) {
	return write28(sector, emptysector);
}
