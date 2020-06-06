#include <kernel/drivers/storage/FS/models/JRAMFS/JRAMFS.hpp>
#include <kernel/klibc/stdlib.hpp>

uint32_t JRAMFS_model::newfile(uint64_t size, uint8_t* data, uint32_t uid, uint8_t filetype, uint16_t permissions) {
	INODE inode;
	inode.n_links = 0;
	inode.size = size;
	inode.creation_time = inode.last_mod_time = inode.last_access_time = 0;
	inode.n_blocks = size / ATA_SECTOR_SIZE;
	if(size % ATA_SECTOR_SIZE) ++inode.n_blocks;
	inode.uid = uid;
	inode.permissions = permissions;
	inode.filetype = filetype;
	inode.flags = 0;

	for(uint32_t i=0; i<inode.n_blocks; ++i) {
		uint32_t thisblock = (uint32_t)(new uint8_t[ATA_SECTOR_SIZE]);

		if(i != inode.n_blocks - 1) {
			memcpy((uint8_t*)thisblock, data, ATA_SECTOR_SIZE);
		} else if(size % ATA_SECTOR_SIZE) {
			memcpy((uint8_t*)thisblock, data, size % ATA_SECTOR_SIZE);
		} else {
			memcpy((uint8_t*)thisblock, data, ATA_SECTOR_SIZE);
		}

		data += ATA_SECTOR_SIZE;
		putBlockInInode(inode, i, thisblock);
	}

	uint32_t ret = this->inodes.size();
	this->inodes[ret] = inode;
	return ret;
}

void JRAMFS_model::appendToFile(uint32_t inode_n, uint64_t size, uint8_t* data) {
	uint64_t appendsize = size;

	INODE& inode = this->inodes[inode_n];

	uint32_t spacestart = inode.size % ATA_SECTOR_SIZE;
	if(spacestart) {
		uint32_t blockid = getSequentialBlock(inode, inode.n_blocks - 1);
		memcpy((uint8_t*)(blockid+spacestart), data, ATA_SECTOR_SIZE - spacestart);
		data += ATA_SECTOR_SIZE - spacestart;
		size -= ATA_SECTOR_SIZE - spacestart;
	}

	uint32_t extrablocks = size / ATA_SECTOR_SIZE;
	if(size % ATA_SECTOR_SIZE) ++extrablocks;

	list<uint32_t> blocks;
	for(uint32_t i=0; i<extrablocks; ++i)
		blocks.push_back((uint32_t)(new uint8_t[ATA_SECTOR_SIZE]));

	for(auto blockid : blocks) {
		memcpy((uint8_t*)blockid, data, ATA_SECTOR_SIZE);
		data += ATA_SECTOR_SIZE;
		size -= ATA_SECTOR_SIZE;
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

		uint16_t upper_bound = ATA_SECTOR_SIZE;
		if(i == inode.n_blocks - 1 && inode.size % ATA_SECTOR_SIZE)
			upper_bound = inode.size % ATA_SECTOR_SIZE;

		memcpy(buffer, (uint8_t*)thisblock, upper_bound);
	}

	return buffer;
}
