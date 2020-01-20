#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.h>
#include <klibc/stdlib.h>

ATA iface;
struct JOTAFS_SUPERBLOCK sb_cache;
uint32_t maxLBA;

JOTAFS::JOTAFS(ATA iface) : iface(iface) {
	uint16_t identifydata[256*2];
	int aux = iface.identify(identifydata);
	status = (aux == 0);
	maxLBA = (identifydata[61] << 16) + identifydata[60];
}

bool JOTAFS::getStatus() { return status; }

uint32_t JOTAFS::getMaxLBA() { return maxLBA; }

uint8_t JOTAFS::writeMBR(uint8_t* mbr) { return iface.write28(0, mbr); }

struct JOTAFS_SUPERBLOCK* JOTAFS::readSB() {
	struct JOTAFS_SUPERBLOCK* p = (struct JOTAFS_SUPERBLOCK*)(iface.read28(1));
	sb_cache = *p;
	return p;
}

uint8_t JOTAFS::writeSB(struct JOTAFS_SUPERBLOCK* sb) {
	sb_cache = *sb;
	return iface.write28(1, (uint8_t*)sb);
}

uint32_t JOTAFS::getFreeLBABlock(void) {
	// Find the first non-full chunk.
	uint32_t offset = 2+sb_cache.s_first_chunk;
	uint8_t* p = 0;
	for(uint32_t i=0; i<sb_cache.n_chunks; i++) {
		p = iface.read28(offset);
		if(!(*p & 0x80)) break;
		jfree(p);
		offset += 512;
	}

	// Find the first unused block in the chunk.
	offset++;
	for(uint16_t i=1; i<512; i++) {
		if(!p[i]) break;
		offset++;
	}
	jfree(p);

	// Clear it.
	iface.clear28(offset);

	return offset;
}

/*
	Mode 0: free
	Mode 1: used
*/
uint8_t JOTAFS::markBlock(uint32_t LBAsector, uint8_t mode) {
	// Get the block and the chunk it's in.
	LBAsector -= 2;
	LBAsector -= sb_cache.s_first_chunk;
	uint16_t block_in_chunk = LBAsector % 512;
	LBAsector /= 512;
	LBAsector = 2+sb_cache.s_first_chunk+(512*LBAsector);

	// Mark it.
	uint8_t* p = iface.read28(LBAsector);
	p[block_in_chunk] = mode;

	// If we're freeing, then the chunk is now not-full.
	if(mode == 0) {
		p[0] = 0;
	} else {
		// Let's check now if all bytes but the first one are ones.
		uint8_t full_chunk = 1;
		for(uint16_t i=1; i<512; i++) {
			if(!p[i]) {
				full_chunk = 0;
				break;
			}
		}
		p[0] = full_chunk;	// Faster than comparisons.
	}

	uint8_t ret = iface.write28(LBAsector, p);
	jfree(p);
	return ret;
}

uint8_t JOTAFS::markBlockAsFree(uint32_t LBAsector) { return markBlock(LBAsector, 0); }
uint8_t JOTAFS::markBlockAsUsed(uint32_t LBAsector) { return markBlock(LBAsector, 1); }

uint32_t JOTAFS::getFreeLBAInode(void) {
	// Find the first inode such that its "isUsed" field is zero.
	uint32_t offset;
	for(offset=2; offset<2+sb_cache.n_inodes; offset++) {
		struct JOTAFS_INODE* inode = (struct JOTAFS_INODE*)(iface.read28(offset));
		uint8_t used = inode->isUsed;
		jfree(inode);
		if(!used) break;
	}
	return offset;
}

void JOTAFS::markInodeAsFree(uint32_t LBAsector) {
	struct JOTAFS_INODE* inode = (struct JOTAFS_INODE*)(iface.read28(LBAsector));
	inode->isUsed = 0;
	iface.write28(LBAsector, (uint8_t*)inode);
	jfree(inode);
}
