#include <kernel/drivers/storage/FS/JRAMFS/JRAMFS.hpp>
#include <kernel/klibc/stdlib.hpp>

JRAMFS::JRAMFS() {
	// Inode 0 is NULL, so must not be assigned. That's pretty much all.
	this->inodes[0] = INODE();
}

inline uint32_t power128(uint32_t exp) { return 1 << (7*exp); }
inline uint32_t div_power128(uint32_t n, uint32_t exp) { return n >> (7*exp); }

uint8_t JRAMFS::getLevel(uint32_t i) {
	// If it's 9 or less, it's a DBP.
	if(i <= 9) return 0;

	// Otherwise, it's an IBP. But which?
	// We'll use some upper bounds to get it.
	for(uint8_t level=1; level<=4; level++)
		if(i <= 9+power128(level))
			return level;

	return 0;	// This will never happen.
}

uint32_t JRAMFS::discardLowerLevels(uint32_t i, uint8_t level) {
	// First, discard the DBPs.
	if(level > 0) i -= JOTAFS_NUMBER_OF_DBPS;

	// Now, get rid of the indirect levels below 'level'.
	// Remember: in IBP 'l' there are 128^l blocks.
	for(uint8_t l=1; l<level; l++) i -= power128(l);

	return i;
}

// Returns the i-th sequential block of an inode.
uint32_t JRAMFS::getSequentialBlock(const INODE& inode, uint32_t i) {
	// Fast comparison.
	if(i <= 9) return inode.DBPs[i];

	uint8_t level = getLevel(i);

	// Now we have to go down the IBPs tree to find the block ID of 'i'.
	i = discardLowerLevels(i, level);

	uint32_t nextBlock = inode.IBPs[level-1];
	while(level) {
		uint32_t* contents = (uint32_t*)nextBlock;	// nextBlock is just a pointer.

		// Get the index of the next level, dividing 'i' by 128^(level-1).
		uint32_t idx = div_power128(i, level-1);
		nextBlock = contents[idx];
		delete [] contents;

		level--;
	}

	return nextBlock;
}

// Puts the i-th sequential block in an inode.
void JRAMFS::putBlockInInode(INODE& inode, uint32_t i, uint32_t block) {
	uint8_t level = getLevel(i);
	if(level == 0) {
		inode.DBPs[i] = block;
		return;
	}

	// Go down the IBPs tree, creating nodes if necessary.
	uint32_t idx = discardLowerLevels(i, level);
	// Is the IBP already there? If not, create it.
	if(!inode.IBPs[level-1]) inode.IBPs[level-1] = (uint32_t)(new uint8_t[BYTES_PER_SECTOR]);
	uint32_t nextBlock = inode.IBPs[level-1];
	while(level) {
		// Get ready to read the contents.
		uint32_t* contents = (uint32_t*)nextBlock;

		// Get the index.
		uint32_t index = div_power128(idx, level-1);

		// Are there any indirect levels left?
		if(level > 1) {
			// Is it already there?
			if(!contents[index]) contents[index] = (uint32_t)(new uint8_t[BYTES_PER_SECTOR]);
		} else {
			// Just put the block we just wrote.
			contents[index] = block;
		}

		memcpy(contents, (uint8_t*)nextBlock, BYTES_PER_SECTOR);
		nextBlock = contents[index];
		level--;
	}
}
