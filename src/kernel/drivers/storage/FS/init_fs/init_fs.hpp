#ifndef PREPARE_RAMFS_HPP
#define PREPARE_RAMFS_HPP

#include <kernel/drivers/storage/FS/JRAMFS>

/*
	Initializes a J*FS filesystem, creating the
	directory structure and such.
*/

void init_fs(VFS* vfs);

#endif
