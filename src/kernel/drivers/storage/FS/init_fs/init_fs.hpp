#ifndef PREPARE_RAMFS_HPP
#define PREPARE_RAMFS_HPP

#include <kernel/drivers/storage/FS/JRAMFS>

#define INIT_FS_UNUSED_PARAMETER 4

/*
	Initializes a J*FS filesystem, creating the
	directory structure and such.
*/

void init_fs(VFS* vfs, uint8_t driveid);

#endif
