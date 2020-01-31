#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.h>
#include <klibc/stdlib.h>

JOTAFS::DIR::DIR(JOTAFS* parent, uint32_t inode_n, void* cached)
	: parent(parent), inode_n(inode_n) {
	if(cached)
		inode_cache = *(JOTAFS_INODE*)cached;
	else
		inode_cache = parent->getInode(inode_n);
}

JOTAFS::DIR::DIR() {}

/*
	This method adds a file to a directory.
	It also defragmentates in case a child has been deleted.
*/
void JOTAFS::DIR::addChild(string filename, uint32_t child_inode_number) {
	// Read the contents.
	uint8_t* old = parent->readWholeFile(inode_n);

	// Allocate some memory.
	uint8_t* contents = new uint8_t[inode_cache.size + filename.length() + 1 + 4];

	// Copy the contents, such that there are no consecutive two null bytes.
	uint8_t lastWasNull = 0;
	uint64_t newsize = 0;
	for(uint64_t i=0; i<inode_cache.size; i++) {
		if(old[i] == 0 && lastWasNull) continue;

		contents[newsize++] = old[i];
		lastWasNull = (old[i] == 0);
	}
	jfree(old);

	// Append the new file.
	for(uint16_t i=0; i<=filename.length(); i++) contents[newsize++] = filename[i];
	for(signed char j=24; j>0; j-=8) contents[newsize++] = (child_inode_number >> j) & 0xFF;

	// Free the blocks.
	uint32_t newsize_in_blocks = newsize / BYTES_PER_SECTOR;
	if(newsize % BYTES_PER_SECTOR) newsize_in_blocks++;
	for(uint32_t i=0; i<newsize_in_blocks; i++) parent->freeBlock(parent->getSequentialBlock(inode_cache, i));

	// "Overwrite" ( ͡° ͜ʖ ͡°)
	uint32_t aux_inode_number = parent->newfile(newsize, contents, 0xDEADBEEF, 0, 0);
	JOTAFS_INODE aux_inode = parent->getInode(aux_inode_number);
	for(uint8_t i=0; i<10; i++) inode_cache.DBPs[i] = aux_inode.DBPs[i];
	for(uint8_t i=0; i<4; i++) inode_cache.IBPs[i] = aux_inode.IBPs[i];

	parent->freeInode(aux_inode_number);
	inode_cache.size = newsize;
	inode_cache.n_blocks = newsize_in_blocks;
	parent->writeInode(inode_n, inode_cache);

	jfree(contents);
}

uint32_t JOTAFS::DIR::getInodeNumber() const { return inode_n; }

JOTAFS::DIR JOTAFS::newdir(uint32_t uid, uint16_t permissions) {
	uint32_t inode_n = newfile(0, 0, uid, JOTAFS_FILETYPE_DIRECTORY, permissions);
	return JOTAFS::DIR(this, inode_n);
}
