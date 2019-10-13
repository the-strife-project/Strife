#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.h>
#include <libc/stdlib.h>

/*
	I recommend listening to "The Strokes - Hard to Explain" right now.

	JOTAFS_updaterecursive handles the insertion of blocks in the inode.
	You should read README.md in this directory first.

	level: 1 (singly indirect block pointer), 2 (doubly), ... ; NON-ZERO
	i: the block number of the file. If it's the first 512 bytes, it's zero, and so on.
	recLBA: recursive LBA. Contains the LBA sector of the current countainer.
			Let's say we're dealing with a singly IBP, when we call it, it will contain
			the LBA sector that contains 128 LBA addresses.
	realLBA: the previously filled block that contains the actual data.
*/
void JOTAFS_updaterecursive(uint8_t level, uint32_t i, uint32_t recLBA, uint32_t realLBA) {
	uint32_t* contents = (uint32_t*)ATA_read28(iface, recLBA);
	uint32_t idx = (i-10) / (1 << (7*level));	// (i-10) / (128**level)

	if(level > 1) {
		// Doubly or more IBP.
		// Is it already set?
		if(!contents[idx]) {
			// Nope, create the level below.
			contents[idx] = JOTAFS_getFreeLBABlock();
			JOTAFS_markBlockAsUsed(contents[idx]);
		}
	} else {
		// Singly IBP. Just put the realLBA.
		contents[idx] = realLBA;
	}

	ATA_write28(iface, recLBA, (uint8_t*)contents);
	jfree(contents);

	// Are we done?
	if(level == 1) return;	// Yep.

	// Nope. Go to the next level!
	JOTAFS_updaterecursive(level-1, i, contents[idx], realLBA);
}

uint32_t JOTAFS_newfile(uint64_t size, uint8_t* data, uint32_t uid, uint8_t exec, uint8_t dir) {
	// 'calloc' is used to fill the area with zeros and not leak data.
	struct JOTAFS_INODE* inode = (struct JOTAFS_INODE*)jcalloc(1, 512);
	inode->size = size;
	// The line below is to be kept until I implement POSIX time.
	inode->creation_time = inode->last_mod_time = inode->last_access_time = 0;
	inode->n_blocks = size / 512;
	if(size % 512) inode->n_blocks++;
	inode->uid = uid;
	inode->isApp = exec;
	inode->isDir = dir;

	// Let's start filling up the blocks.
	uint32_t size_in_blocks = size / 512;
	if(size % 512) size_in_blocks++;

	uint32_t LBA_singly, LBA_doubly, LBA_triply, LBA_quadruply;
	LBA_singly = LBA_doubly = LBA_triply = LBA_quadruply = 0;
	for(uint32_t i=0; i<size_in_blocks; i++) {
		uint32_t thisblock = JOTAFS_getFreeLBABlock();
		if(i != size_in_blocks-1) {
			ATA_write28(iface, thisblock, data);
		} else if(size % 512) {
			/*
				We're on the last block and the file is not padded.
				We now copy the contents to a new memory location,
				and fill the remaining bytes with zeros so no extra
				memory is written.
			*/
			uint8_t* contents = jmalloc(512);
			for(uint16_t i=0; i<size%512; i++) contents[i] = data[i];
			for(uint16_t i=size%512; i<512; i++) contents[i] = 0;
			ATA_write28(iface, thisblock, contents);
			jfree(contents);
		}
		data += 512;
		JOTAFS_markBlockAsUsed(thisblock);

		// 'thisblock' is now set.
		// Put the block in the inode.
		if(i > 9+(128*128*128)) {
			// Quadruply indirect block pointer.
			if(!LBA_quadruply) {
				LBA_quadruply = JOTAFS_getFreeLBABlock();
				JOTAFS_markBlockAsUsed(LBA_quadruply);
				inode->ext_4 = LBA_quadruply;
			}
			JOTAFS_updaterecursive(4, i, LBA_quadruply, thisblock);
		} else if(i > 9+(128*128)) {
			// Triply indirect block pointer.
			if(!LBA_triply) {
				LBA_triply = JOTAFS_getFreeLBABlock();
				JOTAFS_markBlockAsUsed(LBA_triply);
				inode->ext_3 = LBA_triply;
			}
			JOTAFS_updaterecursive(3, i, LBA_triply, thisblock);
		} else if(i > 9+128) {
			// Doubly indirect block pointer.
			if(!LBA_doubly) {
				LBA_doubly = JOTAFS_getFreeLBABlock();
				JOTAFS_markBlockAsUsed(LBA_doubly);
				inode->ext_2 = LBA_doubly;
			}
			JOTAFS_updaterecursive(2, i, LBA_doubly, thisblock);
		} else if(i > 9) {
			// Singly indirect block pointer.
			if(!LBA_singly) {
				LBA_singly = JOTAFS_getFreeLBABlock();
				JOTAFS_markBlockAsUsed(LBA_singly);
				inode->ext_1 = LBA_singly;
			}
			JOTAFS_updaterecursive(1, i, LBA_singly, thisblock);
		} else {
			// Direct.
			inode->DBPs[i] = thisblock;
		}
	}

	// Write the inode.
	uint32_t inode_LBA = JOTAFS_getFreeLBAInode();
	ATA_write28(iface, inode_LBA, (uint8_t*)inode);

	return inode_LBA;
}
