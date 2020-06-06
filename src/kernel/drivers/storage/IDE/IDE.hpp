#ifndef IDE_HPP
#define IDE_HPP

#include <kernel/drivers/PCI/PCI.hpp>
#include <kernel/drivers/storage/FSRawChunk.hpp>

class IDE {
public:
	struct Device {
		uint8_t Reserved;		// 0 (empty), 1 (exists).
		uint8_t Channel;		// 0 (primary), 1 (secondary).
		uint8_t Drive;			// 0 (master), 1 (slave).
		uint16_t Type;			// 0 (ATA), 1 (ATAPI).
		uint16_t Signature;	// Drive signature.
		uint16_t Capabilities;	// Features.
		uint32_t CommandSets;	// Supported command sets.
		uint32_t Size;			// Size in sectors.
		uint8_t Model[41];		// String.
	};

private:
	struct ChannelRegisters {
		uint16_t base;	// I/O base.
		uint16_t ctrl;	// Control base.
		uint16_t bmide; // Bus master IDE
		uint8_t nIEN;	// No interrupt.
	};

	// Do I really care about the PCI device or just the descriptor?
	//PCI pci;
	ChannelRegisters channels[2];

	// Buffer for identification space.
	uint8_t buff[2048];
	// Whether an IRQ has been invoked.
	volatile bool irq_invoked;
	uint8_t atapi_packet[12];

	Device devices[4];

	// Private functions :)
	uint8_t ATAPI_read_sector(uint8_t drive, uint32_t lba, uint8_t* buffer) __attribute__((optimize(0)));
	uint8_t ATA_access(bool write, uint8_t drive, uint32_t lba, uint8_t numsects, uint8_t* buffer) __attribute__((optimize(0)));

public:
	IDE() {}

	void writeReg(uint8_t channel, uint8_t reg, uint8_t data);
	uint8_t readReg(uint8_t channel, uint8_t reg);
	void readBuffer(uint8_t channel, uint8_t reg, uint32_t quads);
	uint8_t polling(uint8_t channel, bool advanced_check);
	uint8_t printError(uint32_t drive, uint8_t err);
	void initialize(uint32_t BAR0, uint32_t BAR1, uint32_t BAR2, uint32_t BAR3, uint32_t BAR4);

	inline Device* getDevices() {
		return devices;
	}

	inline void set_irq() {
		irq_invoked = true;
	}
	inline void wait_irq() {
		while(!irq_invoked) {}
		irq_invoked = false;
	}

	// ATAPI
	FSRawChunk ATAPI_read(uint8_t drive, uint32_t lba, uint32_t count);

	// ATA
	FSRawChunk ATA_read(uint8_t drive, uint32_t lba, uint32_t count);
	void ATA_write(uint8_t drive, uint32_t lba, uint32_t count, uint8_t* contents);
};

extern IDE ide;

extern "C" void IDT_IDE();
extern "C" void ide_irq_handler();



// Big definitions.
#define ATAPI_SECTOR_SIZE 2048
#define ATAPI_WORDS_PER_SECTOR 1024
#define ATA_SECTOR_SIZE 512
#define ATA_WORDS_PER_SECTOR 256
#define IDE1_IRQ 14
#define IDE2_IRQ 15
#define IDE1_IDT_ENTRY 0x2E
#define IDE2_IDT_ENTRY 0x2F

/* STATUS */
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

/* ERRORS */
#define ATA_ER_BBK      0x80    // Bad block
#define ATA_ER_UNC      0x40    // Uncorrectable data
#define ATA_ER_MC       0x20    // Media changed
#define ATA_ER_IDNF     0x10    // ID mark not found
#define ATA_ER_MCR      0x08    // Media change request
#define ATA_ER_ABRT     0x04    // Command aborted
#define ATA_ER_TK0NF    0x02    // Track 0 not found
#define ATA_ER_AMNF     0x01    // No address mark

/* COMMANDS */
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC
#define      ATAPI_CMD_READ       0xA8
#define      ATAPI_CMD_EJECT      0x1B
// Identification space. SHOULD BE MADE A STRUCT!
#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200
// End of identification space.
#define IDE_ATA        0x00
#define IDE_ATAPI      0x01
#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01
#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

// Channels
#define ATA_PRIMARY 0x00
#define ATA_SECONDARY 0x01

// Directions
#define ATA_READ 0x00
#define ATA_WRITE 0x01

#endif
