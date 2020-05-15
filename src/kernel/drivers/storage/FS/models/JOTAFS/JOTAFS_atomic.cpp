#include <kernel/drivers/storage/FS/models/JOTAFS/JOTAFS.hpp>
#include <kernel/klibc/stdlib.hpp>
#include <kernel/klibc/STL/bitmap>
#include <kernel/klibc/stdio>

JOTAFS_model::JOTAFS_model(ATA iface) : iface(iface) {
	uint16_t identifydata[256*2];
	int aux = iface.identify(identifydata);
	status = (aux == 0);
	maxSector = (identifydata[61] << 16) + identifydata[60];

	SUPERBLOCK* p = (SUPERBLOCK*)iface.read28(JOTAFS_SECTOR_SUPERBLOCK);
	sb_cache = *p;
	jfree(p);
}

bool JOTAFS_model::getStatus() { return status; }
uint32_t JOTAFS_model::getMaxSector() { return maxSector; }

uint8_t JOTAFS_model::writeBoot(uint8_t* boot) { return iface.write28(JOTAFS_SECTOR_BOOT, boot); }

uint8_t JOTAFS_model::writeSB(const SUPERBLOCK& sb) {
	sb_cache = sb;
	return iface.write28(JOTAFS_SECTOR_SUPERBLOCK, (uint8_t*)&sb_cache);
}

uint8_t JOTAFS_model::updateSB() {
	return iface.write28(JOTAFS_SECTOR_SUPERBLOCK, (uint8_t*)&sb_cache);
}

JOTAFS_model::INODE JOTAFS_model::getInode(uint32_t idx) {
	INODE* inodes = (INODE*)iface.read28(inode2sector(idx));
	INODE ret = inodes[(idx-1) % JOTAFS_INODES_PER_SECTOR];
	jfree(inodes);
	return ret;
}

void JOTAFS_model::writeInode(uint32_t idx, const INODE& contents) {
	INODE* inodes = (INODE*)iface.read28(inode2sector(idx));
	inodes[(idx-1) % JOTAFS_INODES_PER_SECTOR] = contents;
	iface.write28(inode2sector(idx), (uint8_t*)inodes);
	jfree(inodes);
}



/*
	TODO: Make this allocate a number of blocks instead of one.
	Something like:
		list<uint32_t> allocBlocks(uint32_t count);
	Why? It's way more efficient searching for a given number of free blocks
	than calling this function a bunch of times.
*/
const uint32_t bitmap_size = BYTES_PER_SECTOR*8;
uint32_t JOTAFS_model::allocBlock() {
	// Read the first non-full bitmap.
	uint8_t* raw_bitmap = iface.read28(bitmap2sector(sb_cache.first_non_full_bitmap));

	bitmap bm(bitmap_size, raw_bitmap);
	uint32_t ret = (uint32_t)bm.getFirstZeroAndFlip();

	if(ret == bitmap_size) {
		// This should NEVER happen.
		printf("\n[[[ THE GREAT BUG ]]]\n");
		while(true) {}
	}

	// The current "ret" is an offset of the actual block count.
	ret += bitmap_size * sb_cache.first_non_full_bitmap;

	// Write the new bitmap.
	iface.write28(bitmap2sector(sb_cache.first_non_full_bitmap), raw_bitmap);

	// Time to check if this is still the first non-full bitmap.
	if(bm.getAllOne()) {
		sb_cache.first_non_full_bitmap++;
		updateSB();
	}

	// Clear the contents.
	uint8_t empty[512] = {0};
	writeBlock(ret, empty);

	jfree(raw_bitmap);
	return ret;
}

// TODO: SAME AS ABOVE
void JOTAFS_model::freeBlock(uint32_t idx) {
	/*
		TODO: THINK THIS VERY CAREFULLY WHEN NEEDED.

		I had a really big bug in allocBlock() and the code
		below has it too. Offsets! Line 68. Be careful.
	*/
	if(idx == 0) {}
	printf(" {{ jlxip's fault: tried to free a block }} ");
	while(true) {}
	// Which bitmap does this belong to?
	/*uint32_t bitmap_idx = idx / BYTES_PER_SECTOR;

	// Read
	uint8_t* raw_bitmap = iface.read28(bitmap2sector(bitmap_idx));
	bitmap bm(BYTES_PER_SECTOR, raw_bitmap);
	// Flip
	bm.set(idx % BYTES_PER_SECTOR, false);
	// Write
	iface.write28(bitmap2sector(bitmap_idx), raw_bitmap);

	// Is this below the first non-full bitmap?
	if(bitmap_idx < sb_cache.first_non_full_bitmap) {
		// Is it non-full?
		if(!bm.getAllOne()) {
			// Change it to this one.
			sb_cache.first_non_full_bitmap = bitmap_idx;
			updateSB();
		}
	}

	jfree(raw_bitmap);*/
}


uint32_t JOTAFS_model::allocInode() {
	// Read the front of the queue.
	uint32_t ret = sb_cache.first_free_inode;

	// If first_free_inode is 0, there are no free inodes :(
	if(!ret) return 0;

	BOTH_INODES b;
	b.inode = getInode(ret);
	sb_cache.first_free_inode = b.free_inode.next;
	updateSB();

	return ret;
}

uint32_t JOTAFS_model::allocInodeAndWrite(const INODE& inode) {
	/*
		Similar to above. This one is done for optimizing 'newfile'.
		'allocInode' and 'writeInode' called consecutively require 4
		accesses, and this only 3.
	*/
	uint32_t ret = sb_cache.first_free_inode;
	if(!ret) return 0;

	INODE* inodes = (INODE*)iface.read28(inode2sector(ret));

	BOTH_INODES b;
	b.inode = inodes[(ret-1) % JOTAFS_INODES_PER_SECTOR];
	sb_cache.first_free_inode = b.free_inode.next;

	inodes[(ret-1) % JOTAFS_INODES_PER_SECTOR] = inode;
	iface.write28(inode2sector(ret), (uint8_t*)inodes);

	updateSB();
	jfree(inodes);

	return ret;
}

void JOTAFS_model::freeInode(uint32_t idx) {
	// Put it at the end of the queue.
	BOTH_INODES old;
	old.inode = getInode(sb_cache.last_free_inode);
	old.free_inode.next = idx;

	writeInode(idx, old.inode);
	sb_cache.last_free_inode = idx;
	updateSB();
}
