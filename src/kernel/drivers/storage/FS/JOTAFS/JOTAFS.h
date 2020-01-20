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

class JOTAFS {
private:
	ATA iface;
	struct JOTAFS_SUPERBLOCK sb_cache;
	uint32_t maxLBA;
	bool status;

	// JOTAFS_sectorleve.cpp
	uint8_t markBlock(uint32_t LBAsector, uint8_t mode);

	// JOTAFS_newfile.cpp
	void updaterecursive(uint8_t level, uint32_t i, uint32_t recLBA, uint32_t realLBA);

	// JOTAFS_readwholefile.cpp
	uint32_t getrecursive(uint8_t level, uint32_t i, uint32_t recLBA);

public:
	// JOTAFS_sectorlevel.cpp
	JOTAFS(ATA iface);
	bool getStatus();
	uint32_t getMaxLBA();
	uint8_t writeMBR(uint8_t* mbr);
	struct JOTAFS_SUPERBLOCK* readSB();
	uint8_t writeSB(struct JOTAFS_SUPERBLOCK* sb);
	uint32_t getFreeLBABlock();
	uint8_t markBlockAsUsed(uint32_t LBAsector);
	uint8_t markBlockAsFree(uint32_t LBAsector);
	uint32_t getFreeLBAInode();
	void markInodeAsFree(uint32_t LBAsector);

	// JOTAFS_disklevel.cpp
	void format(void);

	// JOTAFS_newfile.cpp
	uint32_t newfile(uint64_t size, uint8_t* data, uint32_t uid, uint8_t exec, uint8_t dir);

	// JOTAFS_dir.cpp
	uint32_t newdir(uint32_t uid);
	void add2dir(uint32_t LBAinode, char* filename, uint32_t lba);

	// JOTAFS_readwholefile.cpp
	uint32_t gimmetheblocc(struct JOTAFS_INODE* inode, uint32_t i);
	void readwholefile(uint32_t inode, uint8_t* buffer);
	uint8_t* allocate_and_readwholefile(uint32_t inode);
};

#endif
