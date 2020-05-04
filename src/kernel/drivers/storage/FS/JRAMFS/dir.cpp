#include <kernel/drivers/storage/FS/JRAMFS/JRAMFS.hpp>
#include <kernel/klibc/STL/map>

// This part differs a lot from JOTAFS.

JRAMFS::DIR::DIR(JRAMFS* parent, uint32_t inode_n)
	: parent(parent), inode_n(inode_n)
{}

JRAMFS::DIR::DIR() {}

#include <kernel/klibc/stdio>
void JRAMFS::DIR::addChild(string filename, uint32_t child_inode_number) {
	// Pretty trivial.
	map<string, uint32_t>* dir = (map<string, uint32_t>*)(this->parent->inodes[inode_n].DBPs[0]);
	(*dir)[filename] = child_inode_number;
}

list<pair<string, uint32_t>> JRAMFS::DIR::getChildren() const {
	list<pair<string, uint32_t>> ret;
	map<string, uint32_t>* dir = (map<string, uint32_t>*)(this->parent->inodes[inode_n].DBPs[0]);
	for(auto const& x : *dir)
		ret.push_back(x);
	return ret;
}

JRAMFS::DIR JRAMFS::newdir(uint32_t uid, uint16_t permissions, uint32_t parent_inode_number) {
	uint32_t inode_n = newfile(0, 0, uid, FILETYPE::DIRECTORY, permissions);
	this->inodes[inode_n].DBPs[0] = (uint32_t)(new map<string, uint32_t>);
	DIR ret(this, inode_n);

	// Add '.' and '..'
	ret.addChild(".", inode_n);
	ret.addChild("..", parent_inode_number);

	return ret;
}
