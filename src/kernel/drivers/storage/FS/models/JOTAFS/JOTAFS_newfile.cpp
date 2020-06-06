#include <kernel/drivers/storage/FS/models/JOTAFS/JOTAFS.hpp>
#include <kernel/klibc/stdlib.hpp>
#include <kernel/klibc/STL/list>
#include <kernel/klibc/stdio>

uint32_t JOTAFS_model::newfile(uint64_t size, uint8_t* data, uint32_t uid, uint8_t filetype, uint16_t permissions) {
	INODE inode;
	inode.used = 1;
	inode.n_links = 0;	// Links will be created by DIR::addChild.
	inode.size = size;
	// The line below is to be kept until I implement POSIX time.
	inode.creation_time = inode.last_mod_time = inode.last_access_time = 0;
	inode.n_blocks = size / ATA_SECTOR_SIZE;
	if(size % ATA_SECTOR_SIZE) inode.n_blocks++;
	inode.uid = uid;
	inode.permissions = permissions;
	inode.filetype = filetype;
	inode.flags = 0;

	// Initialize all block pointers to zero.
	for(uint8_t i=0; i<JOTAFS_NUMBER_OF_DBPS; ++i)
		inode.DBPs[i] = 0;
	for(uint8_t i=0; i<JOTAFS_NUMBER_OF_IBPS; ++i)
		inode.IBPs[i] = 0;

	// Let's start filling up the blocks.
	uint32_t size_in_blocks = inode.n_blocks;

	for(uint32_t i=0; i<size_in_blocks; i++) {
		uint32_t thisblock = allocBlock();

		if(i != size_in_blocks-1) {
			writeBlock(thisblock, data);
		} else if(size % ATA_SECTOR_SIZE) {
			/*
				We're on the last block and the file is not padded.
				We now copy the contents to a new memory location,
				and fill the remaining bytes with zeros so no extra
				memory is written.
			*/
			uint8_t contents[512] = {0};
			for(uint16_t i=0; i<size % ATA_SECTOR_SIZE; i++) contents[i] = data[i];
			writeBlock(thisblock, contents);
		} else {
			writeBlock(thisblock, data);
		}
		data += ATA_SECTOR_SIZE;

		// 'thisblock' is now set.
		// It's time to put the block in the inode.
		putBlockInInode(inode, i, thisblock);
	}

	return allocInodeAndWrite(inode);
}




void JOTAFS_model::appendToFile(uint32_t inode_n, uint64_t size, uint8_t* data) {
	uint64_t appendsize = size;

	// Read the inode.
	INODE inode = getInode(inode_n);

	// First: is there space in the last block?
	uint32_t spacestart = inode.size % ATA_SECTOR_SIZE;
	if(spacestart) {
		// Yep. Append the bytes we need there.
		// Read the block.
		uint32_t blockid = getSequentialBlock(inode, inode.n_blocks - 1);
		FSRawChunk contents = getBlock(blockid);

		// Start putting the data.
		uint16_t written;
		for(written=0; written < size && written+spacestart < ATA_SECTOR_SIZE; ++written) {
			contents.get()[written+spacestart] = *data;
			++data;
		}
		size -= written;

		// That's done. Write the block.
		writeBlock(blockid, contents.get());
		contents.destroy();
	}

	// How many extra blocks do we need?
	uint32_t extrablocks = size / ATA_SECTOR_SIZE;
	if(size % ATA_SECTOR_SIZE) extrablocks++;

	// Allocate them.
	// TODO: Change this once allocBlock is improved.
	list<uint32_t> blocks;
	for(uint32_t i=0; i<extrablocks; ++i)
		blocks.push_back(allocBlock());

	// Write the data!
	for(auto const& blockid : blocks) {
		uint8_t contents[ATA_SECTOR_SIZE] = {0};
		uint16_t written;
		for(written=0; written < size && written < ATA_SECTOR_SIZE; ++written) {
			contents[written] = *data;
			++data;
		}
		size -= written;

		writeBlock(blockid, contents);
	}

	// Put the new blocks.
	uint32_t i = inode.n_blocks;
	for(auto const& blockid : blocks) {
		putBlockInInode(inode, i, blockid);
		++i;
	}

	// Update the size.
	inode.size += appendsize;
	inode.n_blocks += extrablocks;

	// Write the inode.
	writeInode(inode_n, inode);
}
