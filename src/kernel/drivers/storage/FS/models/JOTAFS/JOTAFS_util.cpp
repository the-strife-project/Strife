#ifndef JOTAFS_UTIL
#define JOTAFS_UTIL

#include <kernel/drivers/storage/FS/models/JOTAFS/JOTAFS.hpp>
#include <kernel/klibc/stdlib.hpp>
#include <kernel/klibc/stdio>

uint32_t JOTAFS_model::sector2inode(uint32_t sector) {
	// Let's count: 0 is for the boot sector, 1 the superblock, 2 is the four (JOTAFS_INODES_PER_SECTOR) first inodes.
	return ((sector - 2) * JOTAFS_INODES_PER_SECTOR) + 1;
}
uint32_t JOTAFS_model::inode2sector(uint32_t inode) {
	// The inverse of above.
	return ((inode - 1) / JOTAFS_INODES_PER_SECTOR) + 2;
}


uint32_t JOTAFS_model::getCountInodeSectors() {
	uint32_t ret = sb_cache.n_inodes / JOTAFS_INODES_PER_SECTOR;
	if(sb_cache.n_inodes % JOTAFS_INODES_PER_SECTOR) ret++;
	return ret;
}
uint32_t JOTAFS_model::sector2bitmap(uint32_t sector) {
	// 0 boot sector, 1 superblock, 2 first 4 inodes, 2+upper(number_inodes/4) last inode, the next first bitmap (number 0).
	return sector - (2 + getCountInodeSectors() + 1);
}
uint32_t JOTAFS_model::bitmap2sector(uint32_t bitmap) {
	return bitmap + 2 + getCountInodeSectors() + 1;
}


uint32_t JOTAFS_model::getNumberOfBitmaps() {
	return sb_cache.s_first_block - sb_cache.s_first_bitmap;
}
uint32_t JOTAFS_model::block2sector(uint32_t block) {
	return sb_cache.s_first_block + block;
}


// These two do not need to be in the class.
inline uint32_t power128(uint32_t exp) { return 1 << (7*exp); }
inline uint32_t div_power128(uint32_t n, uint32_t exp) { return n >> (7*exp); }


uint8_t JOTAFS_model::getLevel(uint32_t i) {
	// If it's 9 or less, it's a DBP.
	if(i < JOTAFS_NUMBER_OF_DBPS)
		return 0;
	i -= JOTAFS_NUMBER_OF_DBPS;

	// Otherwise, it's an IBP. But which?
	// We'll use some upper bounds to get it.
	for(uint8_t level=1; level<=4; level++)
		if(i < power128(level))
			return level;

	// It will never arrive here.
	printf("\n {{ jlxip's fault: this is literally impossible: %d }} ", i);
	while(true) {}
	return 0;
}


uint32_t JOTAFS_model::discardLowerLevels(uint32_t i, uint8_t level) {
	// First, discard the DBPs.
	if(level > 0)
		i -= JOTAFS_NUMBER_OF_DBPS;

	// Now, get rid of the indirect levels below 'level'.
	// Remember: in IBP 'l' there are 128^l blocks.
	for(uint8_t l=1; l<level; l++)
		i -= power128(l);

	return i;
}


uint32_t JOTAFS_model::getSequentialBlock(const INODE& inode, uint32_t i) {	// O(1)
	// Fast comparison.
	if(i <= 9) return inode.DBPs[i];

	uint8_t level = getLevel(i);

	// Now we have to go down the IBPs tree to find the block ID of 'i'.
	i = discardLowerLevels(i, level);

	uint32_t nextBlock = inode.IBPs[level-1];
	while(level) {
		FSRawChunk contents_chunk = getBlock(nextBlock);
		uint32_t* contents = (uint32_t*)contents_chunk.get();

		// Get the index of the next level.
		uint32_t idx = div_power128(i, level-1) % 128;
		nextBlock = contents[idx];
		contents_chunk.destroy();

		--level;
	}

	return nextBlock;
}


void JOTAFS_model::putBlockInInode(INODE& inode, uint32_t i, uint32_t block) {	// O(1)
	uint8_t level = getLevel(i);
	if(level == 0) {
		inode.DBPs[i] = block;
		return;
	}

	// Go down the IBPs tree, creating nodes if necessary.
	i = discardLowerLevels(i, level);
	// Is the IBP already there? If not, create it.
	if(!inode.IBPs[level-1])
		inode.IBPs[level-1] = allocBlock();

	uint32_t nextBlock = inode.IBPs[level-1];
	while(level) {
		// Get ready to read the contents.
		FSRawChunk contents_chunk = getBlock(nextBlock);
		uint32_t* contents = (uint32_t*)contents_chunk.get();

		// Get the index.
		uint32_t idx = div_power128(i, level-1) % 128;

		// Are there any indirect levels left?
		if(level > 1) {
			// Is it already there?
			if(!contents[idx])
				contents[idx] = allocBlock();
		} else {
			// Just put the block we just wrote.
			contents[idx] = block;
		}

		writeBlock(nextBlock, (uint8_t*)contents);
		nextBlock = contents[idx];
		contents_chunk.destroy();

		--level;
	}
}

#endif
