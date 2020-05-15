#include <kernel/drivers/storage/FS/models/JOTAFS/JOTAFS.hpp>
#include <kernel/klibc/stdlib.hpp>

uint32_t JOTAFS_model::findInDirectory(uint32_t inode_n, const string& next) {
	map<string, uint32_t> children(DIR(this, inode_n).getChildren());

	for(auto const& x : children)
		if(x.f == next)
			return x.s;

	return RESERVED_INODE::INULL;
}

uint32_t JOTAFS_model::find(const string& path) {
	uint32_t ret = RESERVED_INODE::ROOT;
	list<string> splitted(path.split('/'));

	for(auto const& x : splitted) {
		if(x == "") continue;

		ret = findInDirectory(ret, x);
		if(ret == RESERVED_INODE::INULL)
			return ret;
	}

	return ret;
}
