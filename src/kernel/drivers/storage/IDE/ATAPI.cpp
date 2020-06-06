#include <kernel/drivers/storage/IDE/IDE.hpp>
#include <kernel/asm.hpp>
#include <kernel/drivers/clock/clock.hpp>

uint8_t IDE::ATAPI_read_sector(uint8_t drive, uint32_t lba, uint8_t* buffer) {
	uint32_t channel = devices[drive].Channel;
	uint8_t slavebit = devices[drive].Drive;
	uint32_t bus = channels[channel].base;
	uint32_t words = ATAPI_WORDS_PER_SECTOR;
	uint8_t err;

	// Enable IRQs
	writeReg(channel, ATA_REG_CONTROL, channels[channel].nIEN = 0x0);

	// Set up SCSI packet
	atapi_packet[0] = ATAPI_CMD_READ;
	atapi_packet[1] = 0x0;
	atapi_packet[2] = (lba >> 24) & 0xFF;
	atapi_packet[3] = (lba >> 16) & 0xFF;
	atapi_packet[4] = (lba >> 8) & 0xFF;
	atapi_packet[5] = lba & 0xFF;
	atapi_packet[6] = 0x0;
	atapi_packet[7] = 0x0;
	atapi_packet[8] = 0x0;
	atapi_packet[9] = 1;	// Number of sectors to read.
	atapi_packet[10] = 0x00;
	atapi_packet[11] = 0x00;

	// Select the drive.
	writeReg(channel, ATA_REG_HDDEVSEL, slavebit << 4);

	// 400 nanoseconds delay.
	for(uint8_t i=0; i<4; ++i)
		readReg(channel, ATA_REG_ALTSTATUS);

	// Inform the controller to use PIO.
	writeReg(channel, ATA_REG_FEATURES, 0);

	// Size of the buffer.
	writeReg(channel, ATA_REG_LBA1, (words * 2) & 0xFF);
	writeReg(channel, ATA_REG_LBA2, (words * 2) >> 8);

	// Send the packet command.
	writeReg(channel, ATA_REG_COMMAND, ATA_CMD_PACKET);

	// Wait for the driver to finish or return an error code.
	err = polling(channel, 1);
	if(err)
		return err;

	// Send packet data.
	irq_invoked = false;
	asm(
		"rep outsw"
		:: "c" (6), "d" (bus), "S" (atapi_packet));

	// Receive data.
	wait_irq();
	err = polling(channel, 1);
	if(err)
		return err;

	asm(
		"rep insw"
		:: "c" (words), "d" (bus), "D" ((uint32_t)buffer));

	// Wait for IRQ.
	wait_irq();
	while(readReg(channel, ATA_REG_STATUS) & (ATA_SR_BSY | ATA_SR_DRQ)) {}

	return 0;
}

FSRawChunk IDE::ATAPI_read(uint8_t drive, uint32_t lba, uint32_t count) {
	// TODO: Handle errors.
	FSRawChunk ret(count * ATAPI_SECTOR_SIZE);
	uint8_t* buffer = ret.get();

	while(count--) {
		ATAPI_read_sector(drive, lba++, buffer);
		buffer += ATAPI_SECTOR_SIZE;
	}

	return ret;
}
