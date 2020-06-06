#include <kernel/drivers/storage/FS/models/JOTAFS/JOTAFS.hpp>
#include <kernel/klibc/stdlib.hpp>

/*
	TODO: This method can be optimized.
	For instance, use a list to save ALL the block IDs,
	which would require WAY less accesses to disk.

	list<uint32_t> JOTAFS_model::getAllBlocks(JOTAFS_INODE inode);
*/

uint8_t* JOTAFS_model::readWholeFile(uint32_t idx, uint8_t* buffer) {
	INODE inode = getInode(idx);
	if(!buffer) buffer = (uint8_t*)jmalloc(inode.size);
	for(uint32_t i=0; i<inode.size; ++i) buffer[i] = 0;	// Does this make sense?

	for(uint32_t i=0; i<inode.n_blocks; i++) {
		// Get the block no. i
		uint32_t thisblock = getSequentialBlock(inode, i);

		// Read it.
		FSRawChunk thisblock_contents = getBlock(thisblock);

		// Append only the required.
		uint16_t upper_bound = ATA_SECTOR_SIZE;
		if(i == inode.n_blocks - 1 && inode.size % ATA_SECTOR_SIZE)
			upper_bound = inode.size % ATA_SECTOR_SIZE;

		memcpy(buffer + i*ATA_SECTOR_SIZE, thisblock_contents.get(), upper_bound);

		thisblock_contents.destroy();
	}

	return buffer;
}
