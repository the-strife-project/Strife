/*
	Source:
	https://github.com/AlexandreRouma/MemeOS/blob/master/libs/kernel/multiboot.h
*/

#ifndef BOOT_H
#define BOOT_H

struct MultibootInfo_t{
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t boot_devices;
	uint32_t cmdline;
	uint32_t mods_count;
	uint32_t mods_addr;
	uint32_t syms_num;
	uint32_t syms_size;
	uint32_t syms_addr;
	uint32_t syms_shndx;
	uint32_t mmap_length;
	uint32_t mmap_addr;
	uint32_t drives_length;
	uint32_t drives_addr;
	uint32_t config_table;
	char boot_loader_name[4];
	uint32_t apm_table;
	uint32_t vbe_control_info;
	uint32_t vbe_mode_info;
	uint16_t vbe_mode;
	uint16_t vbe_interface_seg;
	uint16_t vbe_interface_off;
	uint16_t vbe_interface_len;
	uint64_t framebuffer_addr;
	uint32_t framebuffer_pitch;
	uint32_t framebuffer_width;
	uint32_t framebuffer_height;
};

extern void ASM_KERNEL_END(void);
extern void ASM_STACK_BOTTOM(void);
extern void ASM_STACK_TOP(void);

#endif
