#include <kernel/drivers/storage/IDE/IDE.hpp>
#include <kernel/asm.hpp>
#include <kernel/drivers/clock/clock.hpp>
#include <kernel/kernel_panic/kernel_panic.hpp>

uint8_t IDE::ATA_access(bool write, uint8_t drive, uint32_t lba, uint8_t numsects, uint8_t* buffer) {
	uint8_t lba_mode;	// 0: CHS. 1: LBA28. 2: LBA48.
	bool dma;
	uint8_t lba_io[6];
	uint32_t channel = devices[drive].Channel;
	uint8_t slavebit = devices[drive].Drive;
	uint32_t bus = channels[channel].base;
	uint8_t head, err;

	// Disble IRQs (not needed for PIO).
	writeReg(channel, ATA_REG_CONTROL, channels[channel].nIEN = 0x02);

	if(lba >= 0x1000000) {
		// LBA48.
		lba_mode = 2;
		lba_io[0] = lba & 0x000000FF;
		lba_io[1] = (lba & 0x0000FF00) >> 8;
		lba_io[2] = (lba & 0x00FF0000) >> 16;
		lba_io[3] = (lba & 0xFF000000) >> 24;
		lba_io[4] = 0;
		lba_io[5] = 0;
		head = 0;	// Lower 4 bits of HDDEVSEL not used.
	} else if(devices[drive].Capabilities & 0x200) {
		// LBA28.
		lba_mode = 1;
		lba_io[0] = lba & 0x00000FF;
		lba_io[1] = (lba & 0x000FF00) >> 8;
		lba_io[2] = (lba & 0x0FF0000) >> 16;
		lba_io[3] = 0;
		lba_io[4] = 0;
		lba_io[5] = 0;
		head = (lba & 0xF000000) >> 24;
	} else {
		// CHS? Disgusting. Aborting.
		panic(KP_NO_LBA);
	}

	// Does drive support DMA?
	dma = 0;

	// Wait if the drive is busy.
	while(readReg(channel, ATA_REG_STATUS) & ATA_SR_BSY) {}

	// Select drive.
	writeReg(channel, ATA_REG_HDDEVSEL, 0xE0 | (slavebit << 4) | head);

	// Write parameters.
	if(lba_mode == 2) {
		writeReg(channel, ATA_REG_SECCOUNT1, 0);
		writeReg(channel, ATA_REG_LBA3, lba_io[3]);
		writeReg(channel, ATA_REG_LBA4, lba_io[4]);
		writeReg(channel, ATA_REG_LBA5, lba_io[5]);
	}
	writeReg(channel, ATA_REG_SECCOUNT0, numsects);
	writeReg(channel, ATA_REG_LBA0, lba_io[0]);
	writeReg(channel, ATA_REG_LBA1, lba_io[1]);
	writeReg(channel, ATA_REG_LBA2, lba_io[2]);

	// Select the command and send it.
	uint8_t cmd;
	if(lba_mode == 1 && !dma && !write) cmd = ATA_CMD_READ_PIO;
	if(lba_mode == 2 && !dma && !write) cmd = ATA_CMD_READ_PIO_EXT;
	if(lba_mode == 1 && dma && !write) cmd = ATA_CMD_READ_DMA;
	if(lba_mode == 2 && dma && !write) cmd = ATA_CMD_READ_DMA_EXT;
	if(lba_mode == 1 && !dma && write) cmd = ATA_CMD_WRITE_PIO;
	if(lba_mode == 2 && !dma && write) cmd = ATA_CMD_WRITE_PIO_EXT;
	if(lba_mode == 1 && dma && write) cmd = ATA_CMD_WRITE_DMA;
	if(lba_mode == 2 && dma && write) cmd = ATA_CMD_WRITE_DMA_EXT;
	writeReg(channel, ATA_REG_COMMAND, cmd);

	// Actual R/W.
	uint16_t _numsects = numsects;
	if(!_numsects)
		_numsects = 256;

	if(dma) {
		// TODO DMA :)
	} else {
		if(!write) {
			// PIO read.
			for(uint16_t i=0; i<_numsects; ++i) {
				err = polling(channel, 1);
				if(err)
					return err;

				asm("rep insw"
					:: "c" (ATA_WORDS_PER_SECTOR), "d" (bus), "D" ((uint32_t)buffer));

				buffer += ATA_SECTOR_SIZE;
			}
		} else {
			// PIO write.
			for(uint16_t i=0; i<_numsects; ++i) {
				polling(channel, 0);
				asm(
					"rep outsw"
					:: "c" (ATA_WORDS_PER_SECTOR), "d" (bus), "S" ((uint32_t)buffer));

				buffer += ATA_SECTOR_SIZE;
			}

			if(lba_mode == 1)
				writeReg(channel, ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
			else
				writeReg(channel, ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH_EXT);

			polling(channel, 0);
		}
	}

	return 0;
}



FSRawChunk IDE::ATA_read(uint8_t drive, uint32_t lba, uint32_t count) {
	// ATA limits accesses to 256 at a time, so this is a wrapper around that issue.
	FSRawChunk ret(count * ATA_SECTOR_SIZE);
	uint8_t* buffer = ret.get();

	uint32_t chunkwrites = count >> 8;	// count / 256
	for(uint32_t i=0; i<chunkwrites; ++i) {
		// numsects 0 is 256 sectors.
		ATA_access(false, drive, lba, 0, buffer);
		buffer += 256 * ATA_SECTOR_SIZE;
	}

	uint8_t remaining = count & 0xFF;
	if(remaining)
		ATA_access(false, drive, lba, remaining, buffer);

	return ret;
}

void IDE::ATA_write(uint8_t drive, uint32_t lba, uint32_t count, uint8_t* contents) {
	uint32_t chunkwrites = count >> 8;
	for(uint32_t i=0; i<chunkwrites; ++i) {
		ATA_access(true, drive, lba, 0, contents);
		contents += 256 * ATA_SECTOR_SIZE;
	}

	uint8_t remaining = count & 0xFF;
	if(remaining)
		ATA_access(true, drive, lba, remaining, contents);
}
