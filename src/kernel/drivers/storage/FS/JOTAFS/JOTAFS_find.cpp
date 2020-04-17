#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.hpp>
#include <klibc/stdlib.hpp>

uint32_t JOTAFS::findInDirectory(uint32_t inode_n, const string& next) {
	list<pair<string, uint32_t>> children(DIR(this, inode_n).getChildren());

	for(auto const& x : children)
		if(x.f == next)
			return x.s;

	return RESERVED_INODE::INULL;
}

uint32_t JOTAFS::find(const string& path) {
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
