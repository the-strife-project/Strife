#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.h>
#include <klibc/stdlib.h>

inline uint32_t getPtrDiff(uint8_t* ptr1, uint8_t* ptr2) {
	return ((uint32_t)(ptr1 - ptr2));
}

uint32_t JOTAFS::findInDirectory(uint32_t inode_n, const string& next) {
	INODE inode = getInode(inode_n);
	uint8_t* contents = readWholeFile(inode_n);
	uint8_t* aux_contents = contents;

	while(getPtrDiff(aux_contents, contents) < inode.size) {
		string filename = (char*)aux_contents;
		if(filename == next) {
			// Found it. Get the inode number.
			aux_contents += filename.length() + 1;
			jfree(contents);
			return *(uint32_t*)aux_contents;
		} else {
			aux_contents += filename.length() + 1 + 4;
		}
	}

	// Not found :(
	jfree(contents);
	return RESERVED_INODE::INULL;
}

uint32_t JOTAFS::find(const string& path) {
	string next;
	uint32_t ret = RESERVED_INODE::ROOT;

	auto it = path.begin();
	do {
		if(it == path.end() || *it == JOTAFS_SEPARATOR) {
			// Skip the root, the double '/', and in case 'path' ends with the separator.
			if(next == "") continue;

			// At this point, the file to find in inode 'ret' is 'next'.
			ret = findInDirectory(ret, next);
			if(ret == RESERVED_INODE::INULL)
				return ret;

			next = "";
		} else {
			next += *it;
		}
	} while((it++) != path.end());

	return ret;
}
