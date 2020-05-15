#ifndef COMMON_TO_BOTH_FILESYSTEMS_H
#define COMMON_TO_BOTH_FILESYSTEMS_H

#include <kernel/drivers/storage/FS/VFS.hpp>
#include <kernel/klibc/STL/bitmap>

// Just for the constants. TODO: remove this when implemented users and permissions.
#include <kernel/drivers/storage/FS/models/JOTAFS/JOTAFS.hpp>

/*
	These are functions that are common for both JOTAFS and JRAMFS controllers.
	The filesystems are almost the same, so no need to repeat code.
*/

template<typename T> FSRawChunk common_readFile(T& model, const string& path, bool big) {
	// Damn I really need that DMA.
	if(big) {}

	uint32_t inode_n = model.find(path);
	if(inode_n == T::RESERVED_INODE::INULL)
		return FSRawChunk();

	typename T::INODE inode = model.getInode(inode_n);

	uint8_t* ptr = model.readWholeFile(inode_n);
	return FSRawChunk(ptr, inode.size);
}


template<typename T> void common_readFileTo(T& model, const string& path, FSRawChunk& theto) {
	uint32_t inode_n = model.find(path);
	if(inode_n == T::RESERVED_INODE::INULL) {
		theto.invalidate();
		return;
	}

	model.readWholeFile(inode_n, theto.get());
}


// Local function.
// "/a/b/" -> ("/a/", "b")
pair<string, string> _common_cutlast(const string& path);


const uint32_t tmp_uid = JOTAFS_SUPERUSER_UID;
const uint16_t tmp_permissions = JP_USER | JP_GR | JP_GX | JP_OR | JP_OX;
template<typename T> void common_newdir(T& model, const string& path) {
	// TODO: Change this when users appear and such.
	pair<string, string> lastCut = _common_cutlast(path);

	uint32_t parentInode_n = model.find(lastCut.f);
	uint32_t newdirInode_n = model.newdir(tmp_uid, tmp_permissions, parentInode_n).getInodeNumber();
	typename T::DIR(&model, parentInode_n).addChild(lastCut.s, newdirInode_n);
}


template<typename T> set<string> common_ls(T& model, const string& path) {
	set<string> ret;
	for(auto const& x : model.getdir(path).getChildren())
		ret.insert(x.f);
	return ret;
}


template<typename T> bool common_isSymLink(T& model, const string& path, string& dst) {
	uint32_t inode_n = model.find(path);
	if(inode_n == T::RESERVED_INODE::INULL)
		return false;

	typename T::INODE inode = model.getInode(inode_n);
	if(inode.filetype != T::FILETYPE::SOFT)
		return false;

	// It is a symbolic link. Fast link flag?
	bitmap b(1, &inode.flags);
	if(b.get(0)) {
		// Fast link. Start reading the BPs. This is quick and dirty.
		uint32_t quickndirty[JOTAFS_NUMBER_OF_DBPS + JOTAFS_NUMBER_OF_IBPS] = {0};
		uint32_t* ptr = quickndirty;
		for(uint8_t i=0; i<JOTAFS_NUMBER_OF_DBPS; ++i)
			*(ptr++) = inode.DBPs[i];
		for(uint8_t i=0; i<JOTAFS_NUMBER_OF_IBPS; ++i)
			*(ptr++) = inode.IBPs[i];

		dst = (char*)quickndirty;
	} else {
		// Not a fast link. Read the contents.
		uint8_t* contents = model.readWholeFile(inode_n);
		dst = (char*)contents;
		delete [] contents;
	}

	return true;
}


#include <kernel/klibc/stdio>
const uint8_t max_fast_link_size = (JOTAFS_NUMBER_OF_DBPS + JOTAFS_NUMBER_OF_IBPS)*4;
template<typename T> void common_link(T& model, const string& orig, const string& dst) {
	// TODO: Change this when users appear and such.
	uint32_t inode_n;
	if(dst.length() < max_fast_link_size) {
		// Ho, ho. We can do the thing.
		inode_n = model.newfile(0, nullptr, tmp_uid, T::FILETYPE::SOFT, tmp_permissions);
		typename T::INODE inode = model.getInode(inode_n);
		bitmap b(1, &inode.flags);
		b.set(0, true);

		char quickndirty[max_fast_link_size] = {0};
		for(uint8_t i=0; i<dst.length(); ++i)
			quickndirty[i] = dst[i];

		uint32_t* ptr = (uint32_t*)quickndirty;
		for(uint8_t i=0; i<JOTAFS_NUMBER_OF_DBPS; ++i)
			inode.DBPs[i] = *(ptr++);
		for(uint8_t i=0; i<JOTAFS_NUMBER_OF_IBPS; ++i)
			inode.IBPs[i] = *(ptr++);

		model.writeInode(inode_n, inode);
	} else {
		printf(" {{ jlxip's fault: big symlink has not been tested yet. }} ");
		while(true) {}

		uint8_t* contents = new uint8_t[dst.size()];
		for(uint32_t i=0; i<dst.size(); ++i)
			contents[i] = dst[i];

		inode_n = model.newfile(dst.size(), contents, tmp_uid, T::FILETYPE::SOFT, tmp_permissions);
		typename T::INODE inode = model.getInode(inode_n);
		inode.flags = 0;
		model.writeInode(inode_n, inode);
	}

	pair<string, string> lastCut = _common_cutlast(orig);
	typename T::DIR(&model, model.find(lastCut.f)).addChild(lastCut.s, inode_n);
}


template<typename T> bool common_isDir(T& model, const string& path) {
	uint32_t inode_n = model.find(path);
	if(inode_n == T::RESERVED_INODE::INULL)
		return false;

	typename T::INODE inode(model.getInode(inode_n));
	if(inode.filetype == T::FILETYPE::DIRECTORY)
		return true;

	return false;
}

template<typename T> bool common_isFile(T& model, const string& path) {
	uint32_t inode_n = model.find(path);
	if(inode_n == T::RESERVED_INODE::INULL)
		return false;

	typename T::INODE inode(model.getInode(inode_n));
	if(inode.filetype == T::FILETYPE::DIRECTORY)
		return false;

	return true;
}


template<typename T> void common_newfile(T& model, const string& path, FSRawChunk contents) {
	// TODO: users and permissions you already know how it goes.
	uint32_t inode_n = model.newfile(contents.getSize(), contents.get(), JOTAFS_SUPERUSER_UID, JOTAFS_model::FILETYPE::SYSTEM, 0);

	pair<string, string> lastCut = _common_cutlast(path);
	typename T::DIR(&model, model.find(lastCut.f)).addChild(lastCut.s, inode_n);
}

#endif
