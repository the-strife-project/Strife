#include <kernel/V86/V86.hpp>
#include <kernel/paging/paging.hpp>
#include <kernel/klibc/stdlib.hpp>

void V86(uint8_t intnum, struct regs16_t *regs) {
	// Disable paging.
	goback_paging();

	// Run the interrupt.
	int32(intnum, regs);

	// Enable paging again.
	go_paging();
}

struct dapack {
	uint8_t size;
	uint8_t null;
	uint16_t blkcount;
	uint16_t boffset;
	uint16_t bsegment;
	uint32_t start;
	uint32_t upper_lba_bits;
} __attribute__((packed));

void V86_readSectorFromDrive(uint8_t driveid, uint32_t lba, uint8_t* buffer) {
	// Prepare the dapack.
	//V86_SECTOR_ADDR
	struct dapack* d = (struct dapack*)V86_DAPACK_ADDR;
	d->size = 0x10;
	d->null = 0x00;
	d->blkcount = 1;
	d->boffset = V86_SECTOR_ADDR;
	d->bsegment = 0x0000;
	d->start = lba;
	d->upper_lba_bits = 0x00000000;

	// Prepare the registers for the interrupt.
	struct regs16_t regs;
	regs.ax = 0x4200;
	regs.dx = driveid;
	regs.ds = 0;
	regs.si = V86_DAPACK_ADDR;

	V86(V86_LBA_READ_INT, &regs);

	memcpy(buffer, (uint8_t*)V86_SECTOR_ADDR, 512);
}
