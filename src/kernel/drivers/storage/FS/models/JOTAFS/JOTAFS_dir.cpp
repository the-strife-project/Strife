#include <kernel/drivers/storage/FS/models/JOTAFS/JOTAFS.hpp>
#include <kernel/klibc/stdlib.hpp>

JOTAFS_model::DIR::DIR(JOTAFS_model* parent, uint32_t inode_n, void* cached)
	: parent(parent), inode_n(inode_n) {
	if(cached)
		inode_cache = *(INODE*)cached;
	else
		inode_cache = parent->getInode(inode_n);
}

JOTAFS_model::DIR::DIR() {}

/*
	This method adds a file to a directory.
	It also defragmentates in case a child has been deleted.
*/
void JOTAFS_model::DIR::addChild(string filename, uint32_t child_inode_number) {
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

	// Increment the number of links of child_inode_number.
	INODE inode = parent->getInode(child_inode_number);
	parent->writeInode(child_inode_number, inode);
}

uint32_t JOTAFS_model::DIR::getInodeNumber() const { return inode_n; }

inline uint32_t getPtrDiff(uint8_t* ptr1, uint8_t* ptr2) {
	return ((uint32_t)(ptr1 - ptr2));
}

map<string, uint32_t> JOTAFS_model::DIR::getChildren() const {
	map<string, uint32_t> ret;

	INODE inode = parent->getInode(inode_n);
	uint8_t* children = parent->readWholeFile(inode_n);
	uint8_t* auxchildren = children;

	while((uint32_t)(auxchildren - children) < inode.size) {
		string str = (char*)auxchildren;
		auxchildren += str.length() + 1;

		pair<string, uint32_t> toInsert;
		toInsert.f = str;
		toInsert.s = *(uint32_t*)auxchildren;

		ret.insert(toInsert);
		auxchildren += 4;
	}

	jfree(children);
	return ret;
}

JOTAFS_model::DIR JOTAFS_model::newdir(uint32_t uid, uint16_t permissions, uint32_t parent_inode_number) {
	uint32_t inode_n = newfile(0, 0, uid, FILETYPE::DIRECTORY, permissions);
	DIR ret(this, inode_n);

	// Add '.' and '..'
	ret.addChild(".", inode_n);
	ret.addChild("..", parent_inode_number);

	return ret;
}
