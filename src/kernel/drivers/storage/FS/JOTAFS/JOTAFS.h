#ifndef JOTAFS_H
#define JOTAFS_H

#include <common/types.h>
#include <kernel/drivers/storage/ATA_PIO/ATA_PIO.h>
#include <klibc/string>

#define JOTAFS_SECTOR_BOOT 0
#define JOTAFS_SECTOR_SUPERBLOCK 1
#define JOTAFS_BYTES_PER_INODE 128
#define JOTAFS_INODES_PER_SECTOR 4
#define JOTAFS_FIRST_NON_RESERVED_INODE 4
#define JOTAFS_NUMBER_OF_DBPS 10

// TODO: make these classes inside JOTAFS.
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
	uint8_t filetype;
	uint8_t padding1[33];
} __attribute__((packed));

union JOTAFS_BOTH_INODES {
	uint8_t used;
	JOTAFS_INODE inode;
	JOTAFS_FREE_INODE free_inode;
};

// J(otafs) P(ermissions)
#define JP_UR 0b100000000
#define JP_UW 0b010000000
#define JP_UX 0b001000000
#define JP_GR 0b000100000
#define JP_GW 0b000010000
#define JP_GX 0b000001000
#define JP_OR 0b000000100
#define JP_OW 0b000000010
#define JP_OX 0b000000001

// Shortcuts
#define JP_USER  0b111000000
#define JP_GROUP 0b000111000
#define JP_OTHER 0b000000111

enum JOTAFS_FILETYPES {
	JOTAFS_FILETYPE_REGULAR_FILE,
	JOTAFS_FILETYPE_DIRECTORY,
	JOTAFS_FILETYPE_FIFO,
	JOTAFS_FILETYPE_SUCTION,
	JOTAFS_FILETYPE_SOCKET,
	JOTAFS_FILETYPE_VOLATILE,
	JOTAFS_FILETYPE_HARD,
	JOTAFS_FILETYPE_SOFT
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
	// JOTAFS_atomic.cpp
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
	uint32_t newfile(uint64_t size, uint8_t* data, uint32_t uid, uint8_t filetype, uint16_t permissions);

	// JOTAFS_readwholefile.cpp
	// If no buffer is given, it will allocate memory itself.
	uint8_t* readWholeFile(uint32_t inode, uint8_t* buffer = 0);

	// JOTAFS_dir.cpp
	class DIR {
	private:
		JOTAFS* parent;
		uint32_t inode_n;
		JOTAFS_INODE inode_cache;

		// 'cached' is a JOTAFS_INODE address, in case it's in memory, so there's no need to read it again.
		friend class JOTAFS;
		DIR(JOTAFS* parent, uint32_t inode_n, void* cached=0);
	public:
		DIR();
		void addChild(string filename, uint32_t child_inode_number);
		uint32_t getInodeNumber() const;
	};
	friend class DIR;

	DIR newdir(uint32_t uid, uint16_t permissions);
};

#endif
