#include <kernel/drivers/storage/FS/models/JRAMFS/JRAMFS.hpp>
#include <kernel/klibc/stdlib.hpp>

uint32_t JRAMFS_model::newfile(uint64_t size, uint8_t* data, uint32_t uid, uint8_t filetype, uint16_t permissions) {
	INODE inode;
	inode.n_links = 0;
	inode.size = size;
	inode.creation_time = inode.last_mod_time = inode.last_access_time = 0;
	inode.n_blocks = size / BYTES_PER_SECTOR;
	if(size % BYTES_PER_SECTOR) ++inode.n_blocks;
	inode.uid = uid;
	inode.permissions = permissions;
	inode.filetype = filetype;
	inode.flags = 0;

	for(uint32_t i=0; i<inode.n_blocks; ++i) {
		uint32_t thisblock = (uint32_t)(new uint8_t[BYTES_PER_SECTOR]);

		if(i != inode.n_blocks - 1) {
			memcpy((uint8_t*)thisblock, data, BYTES_PER_SECTOR);
		} else if(size % BYTES_PER_SECTOR) {
			memcpy((uint8_t*)thisblock, data, size % BYTES_PER_SECTOR);
		} else {
			memcpy((uint8_t*)thisblock, data, BYTES_PER_SECTOR);
		}

		data += BYTES_PER_SECTOR;
		putBlockInInode(inode, i, thisblock);
	}

	uint32_t ret = this->inodes.size();
	this->inodes[ret] = inode;
	return ret;
}

void JRAMFS_model::appendToFile(uint32_t inode_n, uint64_t size, uint8_t* data) {
	uint64_t appendsize = size;

	INODE& inode = this->inodes[inode_n];

	uint32_t spacestart = inode.size % BYTES_PER_SECTOR;
	if(spacestart) {
		uint32_t blockid = getSequentialBlock(inode, inode.n_blocks - 1);
		memcpy((uint8_t*)(blockid+spacestart), data, BYTES_PER_SECTOR - spacestart);
		data += BYTES_PER_SECTOR - spacestart;
		size -= BYTES_PER_SECTOR - spacestart;
	}

	uint32_t extrablocks = size / BYTES_PER_SECTOR;
	if(size % BYTES_PER_SECTOR) ++extrablocks;

	list<uint32_t> blocks;
	for(uint32_t i=0; i<extrablocks; ++i)
		blocks.push_back((uint32_t)(new uint8_t[BYTES_PER_SECTOR]));

	for(auto blockid : blocks) {
		memcpy((uint8_t*)blockid, data, BYTES_PER_SECTOR);
		data += BYTES_PER_SECTOR;
		size -= BYTES_PER_SECTOR;
	}

	uint32_t i = inode.n_blocks;
	for(auto blockid : blocks)
		putBlockInInode(inode, i++, blockid);

	// Update size.
	inode.size += appendsize;
	inode.n_blocks += extrablocks;
}

uint8_t* JRAMFS_model::readWholeFile(uint32_t inode_n, uint8_t* buffer) {
	INODE inode = this->inodes[inode_n];

	if(!buffer)
		buffer = new uint8_t[inode.size];

	for(uint32_t i=0; i<inode.size; ++i)
		buffer[i] = 0;

	for(uint32_t i=0; i<inode.n_blocks; ++i) {
		uint32_t thisblock = getSequentialBlock(inode, i);

		uint16_t upper_bound = BYTES_PER_SECTOR;
		if(i == inode.n_blocks - 1 && inode.size % BYTES_PER_SECTOR)
			upper_bound = inode.size % BYTES_PER_SECTOR;

		memcpy(buffer, (uint8_t*)thisblock, upper_bound);
	}

	return buffer;
}
