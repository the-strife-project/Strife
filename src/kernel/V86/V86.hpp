#ifndef REGS_H
#define REGS_H

#include <common/types.hpp>

struct regs16_t {
	uint16_t di, si, bp, sp, bx, dx, cx, ax;
	uint16_t gs, fs, es, ds, eflags;
} __attribute__((packed));

extern "C" void int32(uint8_t intnum, struct regs16_t *regs);
void V86(uint8_t intnum, struct regs16_t *regs);

/*
	Reads a sector using BIOS interrupts.
	This is used exactly ONCE in the code, for getting the UUID of the boot drive when installed.
	I know I could write this in the bootloader and save a few microseconds of bootup,
	but I don't want to touch the bootloader as long as it works. That thing scares me.
*/
#define V86_DAPACK_ADDR 0x7E00
#define V86_SECTOR_ADDR 0x8000
#define V86_LBA_READ_INT 0x13
void V86_readSectorFromDrive(uint8_t driveid, uint32_t lba, uint8_t* buffer);

#endif
