#ifndef JRAMFS_CONTROLLER_HPP
#define JRAMFS_CONTROLLER_HPP

#include <kernel/drivers/storage/FS/VFS.hpp>
#include <kernel/drivers/storage/FS/models/JRAMFS/JRAMFS.hpp>
#include <kernel/drivers/storage/FS/controllers/common.hpp>

class JRAMFS : public VFS {
private:
	JRAMFS_model model;

public:
	inline bool isReadOnly() const override { return false; }
	inline bool isRAM() const override { return true; }

	inline FSRawChunk readFile(const string& path) override {
		return common_readFile<JRAMFS_model>(model, path);
	}

	inline void readFileTo(const string& path, FSRawChunk& theto) override {
		common_readFileTo<JRAMFS_model>(model, path, theto);
	}

	inline void newdir(const string& path) override {
		common_newdir<JRAMFS_model>(model, path);
	}

	inline set<string> ls(const string& path) override {
		return common_ls<JRAMFS_model>(model, path);
	}

	inline bool isSymLink(const string& path, string& dst) override {
		return common_isSymLink<JRAMFS_model>(model, path, dst);
	}

	inline void link(const string& orig, const string& dst) override {
		return common_link<JRAMFS_model>(model, orig, dst);
	}

	inline bool isDir(const string& path) override {
		return common_isDir<JRAMFS_model>(model, path);
	}

	inline bool isFile(const string& path) override {
		return common_isFile<JRAMFS_model>(model, path);
	}

	inline void newfile(const string& path, FSRawChunk contents) override {
		return common_newfile<JRAMFS_model>(model, path, contents);
	}
};

#endif
