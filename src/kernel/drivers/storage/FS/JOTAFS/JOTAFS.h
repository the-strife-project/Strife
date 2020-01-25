#ifndef JOTAFS_H
#define JOTAFS_H

#include <common/types.h>
#include <kernel/drivers/storage/ATA_PIO/ATA_PIO.h>

#define JOTAFS_SECTOR_BOOT 0
#define JOTAFS_SECTOR_SUPERBLOCK 1
#define JOTAFS_BYTES_PER_INODE 128
#define JOTAFS_INODES_PER_SECTOR 4
#define JOTAFS_FIRST_NON_RESERVED_INODE 4
#define JOTAFS_NUMBER_OF_DBPS 10

struct JOTAFS_SUPERBLOCK {
	uint64_t signature;
	uint32_t n_inodes;
	uint32_t n_blocks;
	uint32_t first_free_inode;
	uint32_t last_free_inode;
	uint32_t s_first_bitmap;	// 's_' is for sector.
	uint32_t first_non_full_bitmap;
	uint32_t s_first_block;
	uint8_t padding[476];
} __attribute__((packed));

struct JOTAFS_FREE_INODE {
	uint8_t used;
	uint8_t padding0[3];
	uint32_t next;
	uint8_t padding1[120];
} __attribute__((packed));

struct JOTAFS_INODE {
	uint8_t used;
	uint8_t padding0[7];
	uint64_t size;
	uint32_t creation_time;
	uint32_t last_mod_time;
	uint32_t last_access_time;
	uint32_t n_blocks;
	uint32_t DBPs[10];
	uint32_t IBPs[4];
	uint32_t uid;
	uint16_t permissions;
	uint8_t flags;
	uint8_t padding1[33];
} __attribute__((packed));

union JOTAFS_BOTH_INODES {
	uint8_t used;
	JOTAFS_INODE inode;
	JOTAFS_FREE_INODE free_inode;
};

struct JOTAFS_PERMISSIONS {
	uint8_t UR; uint8_t UW; uint8_t UX;
	uint8_t GR; uint8_t GW; uint8_t GX;
	uint8_t OR; uint8_t OW; uint8_t OX;
	uint8_t padding[7];
} __attribute__((packed));

struct JOTAFS_FLAGS {
	uint32_t filetype;
	uint32_t unused;
};

enum JOTAFS_FILETYPES {
	JOTAFS_FILETYPE_REGULAR_FILE,
	JOTAFS_FILETYPE_DIRECTORY,
	JOTAFS_FILETYPE_FIFO,
	JOTAFS_FILETYPE_SUCTION,
	JOTAFS_FILETYPE_SOCKET,
	JOTAFS_FILETYPE_VOLATILE
};

enum JOTAFS_RESERVED_INODES {
	JOTAFS_INODE_NULL,
	JOTAFS_INODE_JBOOT2,
	JOTAFS_INODE_KERNEL,
	JOTAFS_INODE_ROOT
};

class JOTAFS {
private:
	ATA iface;
	JOTAFS_SUPERBLOCK sb_cache;
	uint32_t maxSector;
	bool status;

	// Some private functions that will make the code way easier to read.
	uint32_t sector2inode(uint32_t sector);
	uint32_t inode2sector(uint32_t inode);
	uint32_t getCountInodeSectors();
	uint32_t sector2bitmap(uint32_t sector);
	uint32_t bitmap2sector(uint32_t bitmap);
	uint32_t getNumberOfBitmaps();
	uint32_t block2sector(uint32_t block);

	// Get the level of a block 'i' in the inode.
	uint8_t getLevel(uint32_t i);

	// Substract to 'i' the number of blocks below 'level'.
	uint32_t discardLowerLevels(uint32_t i, uint8_t level);

	// Returns the i-th sequential block of an inode.
	uint32_t getSequentialBlock(const JOTAFS_INODE& inode, uint32_t i);

public:
	// JOTAFS_sectorlevel.cpp
	JOTAFS(ATA iface);

	bool getStatus();
	uint32_t getMaxSector();

	uint8_t writeBoot(uint8_t* boot);

	inline const JOTAFS_SUPERBLOCK& getSB() const { return sb_cache; }
	inline bool checkSignature() const { return sb_cache.signature == 0x000CACADEBACA000; }
	uint8_t writeSB(const JOTAFS_SUPERBLOCK& sb);
	uint8_t updateSB();

	JOTAFS_INODE getInode(uint32_t idx);
	void writeInode(uint32_t idx, const JOTAFS_INODE& contents);

	inline uint8_t* getBlock(uint32_t idx) { return iface.read28(block2sector(idx)); }
	inline void writeBlock(uint32_t idx, uint8_t* contents) { iface.write28(block2sector(idx), contents); }

	uint32_t allocBlock();
	void freeBlock(uint32_t idx);

	uint32_t allocInode();
	uint32_t allocInodeAndWrite(const JOTAFS_INODE& inode);
	void freeInode(uint32_t idx);

	// JOTAFS_format.cpp. TODO: THIS SHOULD NOT BE A PART OF THE FILESYSTEM.
	void format(void);

	// JOTAFS_newfile.cpp
	uint32_t newfile(uint64_t size, uint8_t* data, uint32_t uid, uint8_t flags, uint16_t permissions);

	// JOTAFS_dir.cpp
	uint32_t newdir(uint32_t uid);
	void add2dir(uint32_t inode, char* filename, uint32_t lba);

	// JOTAFS_readwholefile.cpp
	// If no buffer is given, it will allocate memory itself.
	uint8_t* readWholeFile(uint32_t inode, uint8_t* buffer = 0);
};

#endif
