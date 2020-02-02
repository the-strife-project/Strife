#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.h>
#include <klibc/stdlib.h>

JOTAFS::DIR::DIR(JOTAFS* parent, uint32_t inode_n, void* cached)
	: parent(parent), inode_n(inode_n) {
	if(cached)
		inode_cache = *(INODE*)cached;
	else
		inode_cache = parent->getInode(inode_n);
}

JOTAFS::DIR::DIR() {}

/*
	This method adds a file to a directory.
	It also defragmentates in case a child has been deleted.
*/
void JOTAFS::DIR::addChild(string filename, uint32_t child_inode_number) {
	// Prepare the directory entry.
	uint8_t* dirent = new uint8_t[filename.length() + 1 + 4];
	uint8_t* aux = dirent;
	for(auto const& x : filename) {
		*aux = x;
		++aux;
	}
	*aux = 0; ++aux;
	uint32_t* auxi = (uint32_t*)aux;
	*auxi = child_inode_number;

	// Append to the file.
	parent->appendToFile(inode_n, filename.length() + 1 + 4, dirent);

	jfree(dirent);
}

uint32_t JOTAFS::DIR::getInodeNumber() const { return inode_n; }

JOTAFS::DIR JOTAFS::newdir(uint32_t uid, uint16_t permissions) {
	uint32_t inode_n = newfile(0, 0, uid, FILETYPE::DIRECTORY, permissions);
	return JOTAFS::DIR(this, inode_n);
}
