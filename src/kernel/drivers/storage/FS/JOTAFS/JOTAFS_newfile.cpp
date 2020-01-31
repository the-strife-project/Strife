#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.h>
#include <klibc/stdlib.h>

inline uint32_t div_power128(uint32_t n, uint32_t exp) { return n >> (7*exp); }

uint32_t JOTAFS::newfile(uint64_t size, uint8_t* data, uint32_t uid, uint8_t filetype, uint16_t permissions) {
	JOTAFS_INODE inode;
	inode.used = 1;
	inode.size = size;
	// The line below is to be kept until I implement POSIX time.
	inode.creation_time = inode.last_mod_time = inode.last_access_time = 0;
	inode.n_blocks = size / BYTES_PER_SECTOR;
	if(size % BYTES_PER_SECTOR) inode.n_blocks++;
	inode.uid = uid;
	inode.filetype = filetype;
	inode.permissions = permissions;

	// Let's start filling up the blocks.
	uint32_t size_in_blocks = inode.n_blocks;

	for(uint32_t i=0; i<size_in_blocks; i++) {
		uint32_t thisblock = allocBlock();

		if(i != size_in_blocks-1) {
			writeBlock(thisblock, data);
		} else if(size % BYTES_PER_SECTOR) {
			/*
				We're on the last block and the file is not padded.
				We now copy the contents to a new memory location,
				and fill the remaining bytes with zeros so no extra
				memory is written.
			*/
			uint8_t contents[512] = {0};
			for(uint16_t i=0; i<size % BYTES_PER_SECTOR; i++) contents[i] = data[i];
			writeBlock(thisblock, contents);
		} else {
			writeBlock(thisblock, data);
		}
		data += BYTES_PER_SECTOR;

		// 'thisblock' is now set.
		// It's time to put the block in the inode.
		uint8_t level = getLevel(i);
		if(level == 0) {
			inode.DBPs[i] = thisblock;
			continue;
		}

		// Go down the IBPs tree, creating nodes if necessary.
		uint32_t idx = discardLowerLevels(i, level);
		// Is the IBP already there? If not, create it.
		if(!inode.IBPs[level-1]) inode.IBPs[level-1] = allocBlock();
		uint32_t nextBlock = inode.IBPs[level-1];
		while(level) {
			// Get ready to read the contents.
			uint32_t* contents = (uint32_t*)getBlock(nextBlock);

			// Get the index.
			uint32_t index = div_power128(idx, level-1);

			// Are there any indirect levels left?
			if(level > 1) {
				// Is it already there?
				if(!contents[index]) contents[index] = allocBlock();
			} else {
				// Just put the block we just wrote.
				contents[index] = thisblock;
			}

			writeBlock(nextBlock, (uint8_t*)contents);
			nextBlock = contents[index];
			jfree(contents);

			level--;
		}
	}

	return allocInodeAndWrite(inode);
}
