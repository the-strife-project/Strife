#include <kernel/drivers/storage/FS/models/JRAMFS/JRAMFS.hpp>

// This part differs a lot from JOTAFS.

JRAMFS_model::DIR::DIR(JRAMFS_model* parent, uint32_t inode_n)
	: parent(parent), inode_n(inode_n)
{}

JRAMFS_model::DIR::DIR() {}

void JRAMFS_model::DIR::addChild(string filename, uint32_t child_inode_number) {
	// Pretty trivial.
	map<string, uint32_t>* dir = (map<string, uint32_t>*)(this->parent->inodes[inode_n].DBPs[0]);
	(*dir)[filename] = child_inode_number;
}

map<string, uint32_t> JRAMFS_model::DIR::getChildren() const {
	map<string, uint32_t> ret;

	map<string, uint32_t>* dir = (map<string, uint32_t>*)(this->parent->inodes[inode_n].DBPs[0]);
	for(auto const& x : *dir)
		ret.insert(x);

	return ret;
}

JRAMFS_model::DIR JRAMFS_model::newdir(uint32_t uid, uint16_t permissions, uint32_t parent_inode_number) {
	uint32_t inode_n = newfile(0, 0, uid, FILETYPE::DIRECTORY, permissions);
	this->inodes[inode_n].DBPs[0] = (uint32_t)(new map<string, uint32_t>);
	DIR ret(this, inode_n);

	// Add '.' and '..'
	ret.addChild(".", inode_n);
	ret.addChild("..", parent_inode_number);

	return ret;
}

uint32_t JRAMFS_model::find(const string& path) {
	uint32_t ret = RESERVED_INODE::ROOT;
	list<string> splitted(path.split('/'));

	for(auto const& x : splitted) {
		if(x == "")
			continue;

		ret = DIR(this, ret).getChildren()[x];
		if(ret == RESERVED_INODE::INULL)
			return ret;
	}

	return ret;
}
