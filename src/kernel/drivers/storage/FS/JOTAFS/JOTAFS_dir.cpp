#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.h>
#include <klibc/stdlib.h>

uint32_t JOTAFS::newdir(uint32_t uid) { return newfile(0, 0, uid, 0, 1); }

/*
	This method adds a file to a directory.
	It also defragmentates in case a child has been deleted.

	TODO: MAKE THIS USE A STRING.
*/
void JOTAFS::add2dir(uint32_t inode_number, char* filename, uint32_t lba) {
	JOTAFS_INODE inode = getInode(inode_number);

	// Read the contents.
	uint8_t* old = readWholeFile(inode_number);

	// Allocate some memory.
	uint8_t contents[inode.size + strlen(filename) + 1 + 4];

	// Copy the contents, such that there are no consecutive two null bytes.
	uint8_t lastWasNull = 0;
	uint64_t newsize = 0;
	for(uint64_t i=0; i<inode.size; i++) {
		if(old[i] == 0 && lastWasNull) continue;

		contents[newsize++] = old[i];
		lastWasNull = (old[i] == 0);
	}
	jfree(old);

	// Append the new file.
	for(uint16_t i=0; i<=strlen(filename); i++) contents[newsize++] = filename[i];
	for(signed char j=24; j>0; j-=8) contents[newsize++] = (lba >> j) & 0xFF;

	// Free the blocks.
	uint32_t newsize_in_blocks = newsize / BYTES_PER_SECTOR;
	if(newsize % 512) newsize_in_blocks++;
	for(uint32_t i=0; i<newsize_in_blocks; i++) freeBlock(getSequentialBlock(inode, i));

	// "Overwrite" ( ͡° ͜ʖ ͡°)
	uint32_t aux_inode_number = newfile(newsize, contents, 0xDEADBEEF, 0, 0);
	JOTAFS_INODE aux_inode = getInode(aux_inode_number);
	for(uint8_t i=0; i<10; i++) inode.DBPs[i] = aux_inode.DBPs[i];
	for(uint8_t i=0; i<4; i++) inode.IBPs[i] = aux_inode.IBPs[i];

	aux_inode.used = 0;
	writeInode(aux_inode_number, aux_inode);

	inode.size = newsize;
	writeInode(inode_number, inode);
}
