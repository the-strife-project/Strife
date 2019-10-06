#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.h>
#include <libc/stdlib.h>

struct ATA_INTERFACE* iface;

uint8_t JOTAFS_init(struct ATA_INTERFACE* _iface) {
	iface = _iface;
	return ATA_identify(iface);
}

uint8_t JOTAFS_writeMBR(uint8_t* mbr) {
	return ATA_write28(iface, 0, mbr);
}

struct JOTAFS_SUPERBLOCK* JOTAFS_readSB() {
	uint8_t* contents = ATA_read28(iface, 1);
	return (struct JOTAFS_SUPERBLOCK*)contents;
}

uint8_t JOTAFS_writeSB(struct JOTAFS_SUPERBLOCK* sb) {
	return ATA_write28(iface, 1, (uint8_t*)sb);
}
