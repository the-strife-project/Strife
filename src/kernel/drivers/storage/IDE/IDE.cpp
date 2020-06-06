#include <kernel/drivers/storage/IDE/IDE.hpp>
#include <kernel/asm.hpp>
#include <kernel/klibc/stdio>
#include <kernel/drivers/clock/clock.hpp>
#include <kernel/GDT/GDT.hpp>
#include <kernel/IDT/IDT.hpp>
#include <kernel/drivers/PIC/PIC.hpp>

IDE ide;

// Write to a register.
void IDE::writeReg(uint8_t channel, uint8_t reg, uint8_t data) {
	if(reg > 0x07 && reg < 0x0C)
		writeReg(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
	if(reg < 0x08)
		outb(channels[channel].base + reg - 0x00, data);
	else if(reg < 0x0C)
		outb(channels[channel].base + reg - 0x06, data);
	else if(reg < 0x0E)
		outb(channels[channel].ctrl + reg - 0x0A, data);
	else if(reg < 0x16)
		outb(channels[channel].bmide + reg - 0x0E, data);
	if(reg > 0x07 && reg < 0x0C)
		writeReg(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

// Read a register in a channel.
uint8_t IDE::readReg(uint8_t channel, uint8_t reg) {
	uint8_t result = 0x77;

	if(reg > 0x07 && reg < 0x0C)
		writeReg(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
	if(reg < 0x08)
		result = inb(channels[channel].base + reg - 0x00);
	else if(reg < 0x0C)
		result = inb(channels[channel].base + reg - 0x06);
	else if(reg < 0x0E)
		result = inb(channels[channel].ctrl + reg - 0x0A);
	else if(reg < 0x16)
		result = inb(channels[channel].bmide + reg - 0x0E);
	if(reg > 0x07 && reg < 0x0C)
		writeReg(channel, ATA_REG_CONTROL, channels[channel].nIEN);

	return result;
}

void IDE::readBuffer(uint8_t channel, uint8_t reg, uint32_t quads) {
	if (reg > 0x07 && reg < 0x0C)
		writeReg(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);

	if (reg < 0x08)
		insl(channels[channel].base  + reg - 0x00, (uint32_t*)buff, quads);
	else if (reg < 0x0C)
		insl(channels[channel].base  + reg - 0x06, (uint32_t*)buff, quads);
	else if (reg < 0x0E)
		insl(channels[channel].ctrl  + reg - 0x0A, (uint32_t*)buff, quads);
	else if (reg < 0x16)
		insl(channels[channel].bmide + reg - 0x0E, (uint32_t*)buff, quads);

	if (reg > 0x07 && reg < 0x0C)
		writeReg(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

uint8_t IDE::polling(uint8_t channel, bool advanced_check) {
	// Delay 400 ns for BSY.
	for(uint8_t i=0; i<4; ++i)
		readReg(channel, ATA_REG_ALTSTATUS);

	// Wait for BSY to be cleared.
	while(readReg(channel, ATA_REG_STATUS) & ATA_SR_BSY) {}

	if(advanced_check) {
		uint8_t state = readReg(channel, ATA_REG_STATUS);

		// Check for errors.
		if(state & ATA_SR_ERR)
			return 2;

		// Device fault?
		if(state & ATA_SR_DF)
			return 1;

		// DRQ?
		if((state & ATA_SR_DRQ) == 0)
			return 3;
	}

	return 0;
}

uint8_t IDE::printError(uint32_t drive, uint8_t err) {
	if (err == 0)
		return err;

	printf("IDE:");
	if (err == 1) {
		printf("- Device Fault\n     ");
		err = 19;
	} else if (err == 2) {
		unsigned char st = readReg(devices[drive].Channel, ATA_REG_ERROR);
		if(st & ATA_ER_AMNF) {
			printf("- No Address Mark Found\n     ");
			err = 7;
		}
		if(st & ATA_ER_TK0NF) {
			printf("- No Media or Media Error\n     ");
			err = 3;
		}
		if(st & ATA_ER_ABRT) {
			printf("- Command Aborted\n     ");
			err = 20;
		}
		if(st & ATA_ER_MCR) {
			printf("- No Media or Media Error\n     ");
			err = 3;
		}
		if(st & ATA_ER_IDNF) {
			printf("- ID mark not Found\n     ");
			err = 21;
		}
		if(st & ATA_ER_MC) {
			printf("- No Media or Media Error\n     ");
			err = 3;
		}
		if(st & ATA_ER_UNC) {
			printf("- Uncorrectable Data Error\n     ");
			err = 22;
		}
		if(st & ATA_ER_BBK) {
			printf("- Bad Sectors\n     ");
			err = 13;
		}
	} else if(err == 3) {
		printf("- Reads Nothing\n     ");
		err = 23;
	} else  if (err == 4) {
		printf("- Write Protected\n     ");
		err = 8;
	}

	printf("- [%s %s] %s\n",
		(const char *[]){"Primary", "Secondary"}[devices[drive].Channel], // Use the channel as an index into the array
		(const char *[]){"Master", "Slave"}[devices[drive].Drive], // Same as above, using the drive
		devices[drive].Model);

   return err;
}

void IDE::initialize(uint32_t BAR0, uint32_t BAR1, uint32_t BAR2, uint32_t BAR3, uint32_t BAR4) {
	// Initialize the IDE instance (has not been initialized yet).
	irq_invoked = false;
	IDT_SET_ENT(IDT[IDE1_IDT_ENTRY], 0, _KERNEL_CODESEGMENT, (uint32_t)IDT_IDE, 0);
	IDT_SET_ENT(IDT[IDE2_IDT_ENTRY], 0, _KERNEL_CODESEGMENT, (uint32_t)IDT_IDE, 0);
	pic_enable_irq(IDE1_IRQ);
	pic_enable_irq(IDE2_IRQ);

	uint32_t count = 0;

	// Detect I/O ports with interface IDE controller.
	channels[ATA_PRIMARY].base = (BAR0 & 0xFFFFFFFC) + 0x1F0 * (!BAR0);
	channels[ATA_PRIMARY].ctrl = (BAR0 & 0xFFFFFFFC) + 0x3F6 * (!BAR1);
	channels[ATA_SECONDARY].base = (BAR0 & 0xFFFFFFFC) + 0x170 * (!BAR2);
	channels[ATA_SECONDARY].ctrl = (BAR0 & 0xFFFFFFFC) + 0x376 * (!BAR3);
	channels[ATA_PRIMARY].bmide = (BAR4 & 0xFFFFFFFC) + 0;
	channels[ATA_SECONDARY].bmide = (BAR4 & 0xFFFFFFFC) + 8;

	// Disable IRQs.
	writeReg(ATA_PRIMARY, ATA_REG_CONTROL, 2);
	writeReg(ATA_SECONDARY, ATA_REG_CONTROL, 2);

	// Detect ATA/ATAPI devices.
	for(uint8_t i=0; i<2; ++i) {
		for(uint8_t j=0; j<2; ++j) {
			uint8_t err = 0, type = IDE_ATA, status;
			devices[count].Reserved = 0;

			// Select drive.
			writeReg(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4));
			// Wait 1ms.
			sleep(1);

			// Send ATA identify command.
			writeReg(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
			sleep(1);

			// Polling.
			if(readReg(i, ATA_REG_STATUS) == 0)
				continue;	// No device.

			while(true) {
				status = readReg(i, ATA_REG_STATUS);
				if((status & ATA_SR_ERR)) {
					err = 1;
					break;
				}
				if(!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ))
					break;
			}

			// Probe for ATAPI devices.
			if(err) {
				uint8_t cl = readReg(i, ATA_REG_LBA1);
				uint8_t ch = readReg(i, ATA_REG_LBA2);

				if(cl == 0x14 && ch == 0xEB)
					type = IDE_ATAPI;
				else if(cl == 0x69 && ch == 0x96)
					type = IDE_ATAPI;
				else
					continue;	// What is that?

				writeReg(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
				sleep(1);
			}

			// Read identification space of the device.
			readBuffer(i, ATA_REG_DATA, 128);

			// Read device parameters.
			devices[count].Reserved = 1;
			devices[count].Type = type;
			devices[count].Channel = i;
			devices[count].Drive = j;
			devices[count].Signature = *((uint16_t*)(buff + ATA_IDENT_DEVICETYPE));
			devices[count].Capabilities = *((uint16_t*)(buff + ATA_IDENT_CAPABILITIES));
			devices[count].CommandSets = *((uint32_t*)(buff + ATA_IDENT_COMMANDSETS));

			// Get size.
			if(devices[count].CommandSets & (1 << 26))
				devices[count].Size = *((uint32_t*)(buff + ATA_IDENT_MAX_LBA_EXT));	// LBA48
			else
				devices[count].Size = *((uint32_t*)(buff + ATA_IDENT_MAX_LBA));	// LBA28

			// Model of device (string)
			for(uint32_t k=0; k<40; k+=2) {
				devices[count].Model[k] = buff[ATA_IDENT_MODEL + k + 1];
				devices[count].Model[k+1] = buff[ATA_IDENT_MODEL + k];
			}
			devices[count].Model[40] = 0;

			++count;
		}
	}
}

extern "C" void ide_irq_handler() {
	outb(PIC_IO_PIC2, PIC_EOI);
	outb(PIC_IO_PIC1, PIC_EOI);
	ide.set_irq();
}
