#ifndef JOTAFS_CONTROLLER_HPP
#define JOTAFS_CONTROLLER_HPP

#include <kernel/drivers/storage/FS/VFS.hpp>
#include <kernel/drivers/storage/FS/models/JOTAFS/JOTAFS.hpp>
#include <kernel/drivers/storage/FS/controllers/common.hpp>

class JOTAFS : public VFS {
private:
	JOTAFS_model model;

public:
	inline bool isReadOnly() const override { return false; }
	inline bool isRAM() const override { return false; }

	JOTAFS(ATA iface) : model(JOTAFS_model(iface)) {}

	inline FSRawChunk readFile(const string& path, bool big=false) override {
		return common_readFile<JOTAFS_model>(model, path, big);
	}

	inline void readFileTo(const string& path, FSRawChunk& theto) override {
		common_readFileTo<JOTAFS_model>(model, path, theto);
	}

	inline void newdir(const string& path) override {
		common_newdir<JOTAFS_model>(model, path);
	}

	inline set<string> ls(const string& path) override {
		return common_ls<JOTAFS_model>(model, path);
	}

	inline bool isSymLink(const string& path, string& dst) override {
		return common_isSymLink<JOTAFS_model>(model, path, dst);
	}

	inline void link(const string& orig, const string& dst) override {
		return common_link<JOTAFS_model>(model, orig, dst);
	}

	inline bool isDir(const string& path) override {
		return common_isDir<JOTAFS_model>(model, path);
	}

	inline bool isFile(const string& path) override {
		return common_isFile<JOTAFS_model>(model, path);
	}

	inline void newfile(const string& path, FSRawChunk contents) override {
		return common_newfile<JOTAFS_model>(model, path, contents);
	}
};

#endif
