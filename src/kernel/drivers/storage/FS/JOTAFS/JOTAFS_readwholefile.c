#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.h>
#include <libc/stdlib.h>

// This method is kind of similar to JOTAFS_updaterecursive, but way simpler.
uint32_t JOTAFS_getrecursive(uint8_t level, uint32_t i, uint32_t recLBA) {
	uint32_t* contents = (uint32_t*)ATA_read28(iface, recLBA);
	uint32_t idx = (i-10) / (1 << (7*level));	// (i-10) / (128**level)

	uint32_t next_recLBA = contents[idx];
	jfree(contents);

	if(level > 1) {
		return JOTAFS_getrecursive(level-1, i, next_recLBA);
	} else {
		return next_recLBA;
	}
}

uint32_t JOTAFS_gimmetheblocc(struct JOTAFS_INODE* inode, uint32_t i) {
	if(i > 9+(128*128*128)) {
		// Quadruply indirect block pointer.
		return JOTAFS_getrecursive(4, i, inode->ext_4);
	} else if(i > 9+(128*128)) {
		// Triply indirect block pointer.
		return JOTAFS_getrecursive(3, i, inode->ext_3);
	} else if(i > 9+128) {
		// Doubly indirect block pointer.
		return JOTAFS_getrecursive(2, i, inode->ext_2);
	} else if(i > 9) {
		// Singly indirect block pointer.
		return JOTAFS_getrecursive(1, i, inode->ext_1);
	} else {
		// Direct.
		return inode->DBPs[i];
	}
}

// Similar as well to JOTAFS_newfile, but a piece of cake.
uint8_t* JOTAFS_readwholefile(uint32_t LBAinode) {
	struct JOTAFS_INODE* inode = (struct JOTAFS_INODE*)ATA_read28(iface, LBAinode);
	uint8_t* data = jmalloc(inode->size);

	uint32_t size_in_blocks = inode->size / 512;
	if(inode->size % 512) size_in_blocks++;

	for(uint32_t i=0; i<size_in_blocks; i++) {
		// Get the block no. i
		uint32_t thisblock = JOTAFS_gimmetheblocc(inode, i);

		// Read it.
		uint8_t* thisblock_contents = ATA_read28(iface, thisblock);

		// Append only the required.
		uint16_t upper_bound = (i != size_in_blocks-1) ? 512 : (inode->size % 512);
		for(uint16_t j=0; j<upper_bound; j++) data[(i*512) + j] = thisblock_contents[j];
		jfree(thisblock_contents);
	}

	jfree(inode);
	return data;
}
