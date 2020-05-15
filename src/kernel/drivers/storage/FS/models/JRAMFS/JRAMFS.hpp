#ifndef JRAMFS_HPP
#define JRAMFS_HPP

#include <kernel/drivers/storage/FS/models/JOTAFS/JOTAFS.hpp>
#include <kernel/klibc/STL/map>

/*
	JRAMFS is basically a hack on JOTAFS.
	Please read the wiki entry about this filesystem.
*/

class JRAMFS_model {
public:
	// Inode, filetypes, and flags are kept.
	typedef JOTAFS_model::INODE INODE;
	typedef JOTAFS_model::FILETYPE FILETYPE;

	struct RESERVED_INODE {
		enum {
			INULL,
			ROOT
		};
	};

	typedef JOTAFS_model::FLAG FLAG;

private:
	map<uint32_t, INODE> inodes;

	// Get the level of a block 'i' in the inode.
	uint8_t getLevel(uint32_t i);

	// Substract to 'i' the number of blocks below 'level'.
	uint32_t discardLowerLevels(uint32_t i, uint8_t level);

	// Returns the i-th sequential block of an inode.
	uint32_t getSequentialBlock(const INODE& inode, uint32_t i);

	// Puts the i-th sequential block in an inode.
	void putBlockInInode(INODE& inode, uint32_t i, uint32_t block);

public:
	// This is done in 'util.cpp', I don't want to create a new file just for that.
	JRAMFS_model();

	inline INODE getInode(uint32_t idx) {
		return inodes[idx];
	}
	inline void writeInode(uint32_t idx, const INODE& contents) {
		inodes[idx] = contents;
	}

	uint32_t newfile(uint64_t size, uint8_t* data, uint32_t uid, uint8_t filetype, uint16_t permissions);
	void appendToFile(uint32_t inode_n, uint64_t size, uint8_t* data);

	uint8_t* readWholeFile(uint32_t inode, uint8_t* buffer = 0);

	class DIR {
	private:
		JRAMFS_model* parent;
		uint32_t inode_n;
		INODE inode_cache;
	public:
		DIR();
		DIR(JRAMFS_model* parent, uint32_t inode_n);

		inline uint32_t getInodeNumber() const {
			return inode_n;
		}
		void addChild(string filename, uint32_t child_inode_number);
		map<string, uint32_t> getChildren() const;
	};

	DIR newdir(uint32_t uid, uint16_t permissions, uint32_t parent_inode_number);

	uint32_t find(const string& path);

	inline DIR getdir(const string& path) {
		return DIR(this, find(path));
	}
};

#endif
