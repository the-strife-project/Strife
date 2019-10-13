#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.h>
#include <libc/stdlib.h>

uint32_t JOTAFS_newdir(uint32_t uid) { return JOTAFS_newfile(0, 0, uid, 0, 1); }

/*
	This method adds a file as a child to a directory.
	It also removes fragmentation in case a child has been deleted.
*/
void JOTAFS_add2dir(uint32_t LBAinode, char* filename, uint32_t lba) {
	struct JOTAFS_INODE* inode = (struct JOTAFS_INODE*)ATA_read28(iface, LBAinode);

	// Read the contents.
	uint8_t* old = JOTAFS_readwholefile(LBAinode);

	// Allocate some memory.
	uint8_t* contents = jmalloc(inode->size + strlen(filename) + 1 + 4);

	// Copy the contents, such that there are no consecutive two null bytes.
	uint8_t lastWasNull = 0;
	uint64_t newsize = 0;
	for(uint64_t i=0; i<inode->size; i++) {
		if(old[i] == 0 && lastWasNull) continue;

		contents[newsize++] = old[i];
		lastWasNull = (old[i] == 0);
	}
	jfree(old);

	// Append the new file.
	for(uint16_t i=0; i<=strlen(filename); i++) contents[newsize++] = filename[i];
	for(signed char j=24; j>0; j-=8) contents[newsize++] = (lba >> j) & 0xFF;

	// Free the blocks.
	uint32_t newsize_in_blocks = newsize / 512;
	if(newsize % 512) newsize_in_blocks++;
	for(uint32_t i=0; i<newsize_in_blocks; i++)
		JOTAFS_markBlockAsFree(JOTAFS_gimmetheblocc(inode, i));

	// "Overwrite" ( ͡° ͜ʖ ͡°)
	uint32_t aux_inode = JOTAFS_newfile(newsize, contents, 0xDEADBEEF, 0, 0);
	struct JOTAFS_INODE* real_aux_inode = (struct JOTAFS_INODE*)ATA_read28(iface, aux_inode);
	for(uint8_t i=0; i<10; i++) inode->DBPs[i] = real_aux_inode->DBPs[i];
	inode->ext_1 = real_aux_inode->ext_1;
	inode->ext_2 = real_aux_inode->ext_2;
	inode->ext_3 = real_aux_inode->ext_3;
	inode->ext_4 = real_aux_inode->ext_4;
	real_aux_inode->isUsed = 0;
	ATA_write28(iface, aux_inode, (uint8_t*)real_aux_inode);
	jfree(real_aux_inode);

	inode->size = newsize;
	ATA_write28(iface, LBAinode, (uint8_t*)inode);
	jfree(inode);
}
