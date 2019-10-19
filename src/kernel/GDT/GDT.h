#ifndef GDT_H
#define GDT_H

#include <common/types.h>

struct GDT_ptr {
	uint16_t size;
	uint32_t base;
} __attribute__((packed));

uint64_t createGDTEntry(uint32_t base, uint32_t limit, uint16_t abf);
void gdt_init(void);

#define _GDT_NULL 0
#define _KERNEL_CODESEGMENT_N 1
#define _KERNEL_DATASEGMENT_N 2
#define _USER_CODESEGMENT_N 3
#define _USER_DATASEGMENT_N 4
#define _TSS_SEGMENT_N 5
#define GDT_ENTRIES 6

#define _KERNEL_CODESEGMENT (_KERNEL_CODESEGMENT_N << 3)
#define _KERNEL_DATASEGMENT (_KERNEL_DATASEGMENT_N << 3)
#define _USER_CODESEGMENT (_USER_CODESEGMENT_N << 3)
#define _USER_DATASEGMENT (_USER_DATASEGMENT_N << 3)
#define _TSS_SEGMENT (_TSS_SEGMENT_N << 3)

// Source: https://wiki.osdev.org/GDT_Tutorial
#define SEG_DESCTYPE(x)  ((x) << 0x04) // Descriptor type (0 for system, 1 for code/data)
#define SEG_PRES(x)      ((x) << 0x07) // Present
#define SEG_SAVL(x)      ((x) << 0x0C) // Available for system use
#define SEG_LONG(x)      ((x) << 0x0D) // Long mode
#define SEG_SIZE(x)      ((x) << 0x0E) // Size (0 for 16-bit, 1 for 32)
#define SEG_GRAN(x)      ((x) << 0x0F) // Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB)
#define SEG_PRIV(x)     (((x) &  0x03) << 0x05)   // Set privilege level (0 - 3)

#define SEG_DATA_RD        0x00 // Read-Only
#define SEG_DATA_RDA       0x01 // Read-Only, accessed
#define SEG_DATA_RDWR      0x02 // Read/Write
#define SEG_DATA_RDWRA     0x03 // Read/Write, accessed
#define SEG_DATA_RDEXPD    0x04 // Read-Only, expand-down
#define SEG_DATA_RDEXPDA   0x05 // Read-Only, expand-down, accessed
#define SEG_DATA_RDWREXPD  0x06 // Read/Write, expand-down
#define SEG_DATA_RDWREXPDA 0x07 // Read/Write, expand-down, accessed
#define SEG_CODE_EX        0x08 // Execute-Only
#define SEG_CODE_EXA       0x09 // Execute-Only, accessed
#define SEG_CODE_EXRD      0x0A // Execute/Read
#define SEG_CODE_EXRDA     0x0B // Execute/Read, accessed
#define SEG_CODE_EXC       0x0C // Execute-Only, conforming
#define SEG_CODE_EXCA      0x0D // Execute-Only, conforming, accessed
#define SEG_CODE_EXRDC     0x0E // Execute/Read, conforming
#define SEG_CODE_EXRDCA    0x0F // Execute/Read, conforming, accessed

#endif
