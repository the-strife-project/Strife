#ifndef JOTAFS_H
#define JOTAFS_H

#include <common/types.h>
#include <kernel/drivers/storage/ATA_PIO/ATA_PIO.h>

struct JOTAFS_SUPERBLOCK {
	uint64_t signature;
	uint32_t n_inodes;
	uint32_t n_chunks;
	uint32_t n_first_unallocated_inode;
	uint32_t s_first_inode;
	uint32_t s_first_chunk;
} __attribute__((packed));

struct JOTAFS_INODE {
	uint64_t size;
	uint32_t creation_time;
	uint32_t last_mod_time;
	uint32_t last_access_time;
	uint32_t n_blocks;
	uint32_t DBPs[10];
	uint32_t ext_1;
	uint32_t ext_2;
	uint32_t ext_3;
	uint32_t ext_4;
	uint32_t uid;
	uint8_t isApp;
	uint8_t isDir;
	uint8_t isUsed;
} __attribute__((packed));

enum JOTAFS_RESERVED_INODES {
	JOTAFS_INODE_JBOOT2,
	JOTAFS_INODE_KERNEL,
	JOTAFS_INODE_ROOT
};

extern struct ATA_INTERFACE* iface;
extern struct JOTAFS_SUPERBLOCK sb_cache;
extern uint32_t maxLBA;

// JOTAFS_sectorlevel.c
uint8_t JOTAFS_init(struct ATA_INTERFACE* iface);
uint32_t JOTAFS_getMaxLBA(void);
uint8_t JOTAFS_writeMBR(uint8_t* mbr);
struct JOTAFS_SUPERBLOCK* JOTAFS_readSB();
uint8_t JOTAFS_writeSB(struct JOTAFS_SUPERBLOCK* sb);
uint32_t JOTAFS_getFreeLBABlock(void);
uint8_t JOTAFS_markBlockAsUsed(uint32_t LBAsector);
uint8_t JOTAFS_markBlockAsFree(uint32_t LBAsector);
uint32_t JOTAFS_getFreeLBAInode(void);
void JOTAFS_markInodeAsFree(uint32_t LBAsector);

// JOTAFS_disklevel.c
void JOTAFS_format(void);

// JOTAFS_newfile.c
uint32_t JOTAFS_newfile(uint64_t size, uint8_t* data, uint32_t uid, uint8_t exec, uint8_t dir);

// JOTAFS_dir.c
uint32_t JOTAFS_newdir(uint32_t uid);
void JOTAFS_add2dir(uint32_t LBAinode, char* filename, uint32_t lba);

// JOTAFS_readwholefile.c
uint32_t JOTAFS_gimmetheblocc(struct JOTAFS_INODE* inode, uint32_t i);
void JOTAFS_readwholefile(uint32_t inode, uint8_t* buffer);
uint8_t* JOTAFS_allocate_and_readwholefile(uint32_t inode);

#endif
