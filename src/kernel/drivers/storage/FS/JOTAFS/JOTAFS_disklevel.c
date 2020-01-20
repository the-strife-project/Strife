#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.h>

void JOTAFS::format(void) {
	// Let's create a superblock and write it into the second sector.
	struct JOTAFS_SUPERBLOCK sb;
	sb.signature = 0x000CACADEBACA000;
	/*
		I have no idea what number of inodes to choose.
		Let's just say, for instance, that 1/32th of the
		total size will be dedicated to inodes.
		In that case, we get the number of inodes as the total size
		divided by 512*32.
	*/
	sb.n_inodes = (getMaxLBA()-2) >> 5;
	/*
		The number of LBA sectors left is MaxLBA - (2 + n_inodes).
		We just divide that by 512 (number of LBA sectors in a chunk.
	*/
	sb.n_chunks = (getMaxLBA() - (2 + sb.n_inodes)) >> 9;
	sb.n_first_unallocated_inode = 0;	// No inodes at formatting time.
	sb.s_first_inode = 2;	// Reserved.
	sb.s_first_chunk = 2+sb.n_inodes;

	// Write it to disk.
	writeSB(&sb);

	// Initialize the inodes.
	for(uint32_t i=0; i<sb.n_inodes; i++) { iface.clear28(2+i); }

	// Initialize the chunks.
	for(uint32_t i=0; i<sb.n_chunks; i++) iface.clear28(sb.s_first_chunk + i*512);
}
