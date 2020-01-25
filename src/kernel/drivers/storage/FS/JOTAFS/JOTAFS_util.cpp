#ifndef JOTAFS_UTIL
#define JOTAFS_UTIL

#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.h>

uint32_t JOTAFS::sector2inode(uint32_t sector) {
	// Let's count: 0 is for the boot sector, 1 the superblock, 2 is the four (JOTAFS_INODES_PER_SECTOR) first inodes.
	return ((sector - 2) * JOTAFS_INODES_PER_SECTOR) + 1;
}
uint32_t JOTAFS::inode2sector(uint32_t inode) {
	// The inverse of above.
	return ((inode - 1) / JOTAFS_INODES_PER_SECTOR) + 2;
}


uint32_t JOTAFS::getCountInodeSectors() {
	uint32_t ret = sb_cache.n_inodes / JOTAFS_INODES_PER_SECTOR;
	if(sb_cache.n_inodes % JOTAFS_INODES_PER_SECTOR) ret++;
	return ret;
}
uint32_t JOTAFS::sector2bitmap(uint32_t sector) {
	// 0 boot sector, 1 superblock, 2 first 4 inodes, 2+upper(number_inodes/4) last inode, the next first bitmap (number 0).
	return sector - (2 + getCountInodeSectors() + 1);
}
uint32_t JOTAFS::bitmap2sector(uint32_t bitmap) {
	return bitmap + 2 + getCountInodeSectors() + 1;
}


uint32_t JOTAFS::getNumberOfBitmaps() {
	return sb_cache.s_first_block - sb_cache.s_first_bitmap;
}
uint32_t JOTAFS::block2sector(uint32_t block) {
	return sb_cache.s_first_block + block;
}


// These two do not need to be in the class.
inline uint32_t power128(uint32_t exp) { return 1 << (7*exp); }
inline uint32_t div_power128(uint32_t n, uint32_t exp) { return n >> (7*exp); }


uint8_t JOTAFS::getLevel(uint32_t i) {
	// If it's 9 or less, it's a DBP.
	if(i <= 9) return 0;

	// Otherwise, it's an IBP. But which?
	// We'll use some upper bounds to get it.
	for(uint8_t level=1; level<=4; level++)
		if(i <= 9+power128(level))
			return level;

	return 0;	// This will never happen.
}


uint32_t JOTAFS::discardLowerLevels(uint32_t i, uint8_t level) {
	// First, discard the DBPs.
	if(level > 0) i -= JOTAFS_NUMBER_OF_DBPS;

	// Now, get rid of the indirect levels below 'level'.
	// Remember: in IBP 'l' there are 128^l blocks.
	for(uint8_t l=1; l<level; l++) i -= power128(l);

	return i;
}


uint32_t JOTAFS::getSequentialBlock(const JOTAFS_INODE& inode, uint32_t i) {	// O(1)
	// Fast comparison.
	if(i <= 9) return inode.DBPs[i];

	uint8_t level = getLevel(i);

	// Now we have to go down the IBPs tree to find the block ID of 'i'.
	i = discardLowerLevels(i, level);

	uint32_t nextBlock = inode.IBPs[level-1];
	while(level) {
		uint32_t* contents = (uint32_t*)getBlock(nextBlock);

		// Get the index of the next level, dividing 'i' by 128^(level-1).
		uint32_t idx = div_power128(i, level-1);
		nextBlock = contents[idx];
		delete [] contents;

		level--;
	}

	return nextBlock;
}

#endif