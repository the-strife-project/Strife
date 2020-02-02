#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.h>
#include <klibc/stdlib.h>

/*
	TODO: This method can be optimized.
	For instance, use a list to save ALL the block IDs,
	which would require WAY less accesses to disk.

	list<uint32_t> JOTAFS::getAllBlocks(JOTAFS_INODE inode);
*/

uint8_t* JOTAFS::readWholeFile(uint32_t idx, uint8_t* buffer) {
	INODE inode = getInode(idx);
	if(!buffer) buffer = (uint8_t*)jmalloc(inode.size);
	for(uint32_t i=0; i<inode.size; ++i) buffer[i] = 0;

	for(uint32_t i=0; i<inode.n_blocks; i++) {
		// Get the block no. i
		uint32_t thisblock = getSequentialBlock(inode, i);

		// Read it.
		uint8_t* thisblock_contents = getBlock(thisblock);

		// Append only the required. TODO: Use memmove.
		uint16_t upper_bound = (i != inode.n_blocks - 1) ? BYTES_PER_SECTOR : (inode.size % BYTES_PER_SECTOR);
		for(uint16_t j=0; j<upper_bound; j++) buffer[(i*BYTES_PER_SECTOR) + j] = thisblock_contents[j];

		jfree(thisblock_contents);
	}

	return buffer;
}
