#ifndef JOTAFS_H
#define JOTAFS_H

#include <common/types.h>
#include <kernel/drivers/storage/ATA_PIO/ATA_PIO.h>

struct JOTAFS_SUPERBLOCK {
	uint64_t signature;
	uint32_t n_inodes;
	uint32_t n_chunks;
	uint32_t s_first_inode;
	uint32_t s_first_chunk;
} __attribute__((packed));

struct JOTAFS_INODE {
	uint64_t size;
	uint32_t cration_time;
	uint32_t last_mod_time;
	uint32_t last_access_time;
	uint32_t n_blocks;
	uint32_t DBP0;
	uint32_t DBP1;
	uint32_t DBP2;
	uint32_t DBP3;
	uint32_t DBP4;
	uint32_t DBP5;
	uint32_t DBP6;
	uint32_t DBP7;
	uint32_t DBP8;
	uint32_t DBP9;
	uint32_t ext_1;
	uint32_t ext_2;
	uint32_t ext_3;
	uint32_t ext_4;
	uint32_t uid;
	uint8_t isApp;
	uint8_t isDir;
} __attribute__((packed));

enum JOTAFS_RESERVED_INODES {
	JOTAFS_INODE_JBOOT2,
	JOTAFS_INODE_KERNEL,
	JOTAFS_INODE_ROOT
};

uint8_t JOTAFS_init(struct ATA_INTERFACE* iface);
uint8_t JOTAFS_writeMBR(uint8_t* mbr);
struct JOTAFS_SUPERBLOCK* JOTAFS_readSB();
uint8_t JOTAFS_writeSB(struct JOTAFS_SUPERBLOCK* sb);

#endif
