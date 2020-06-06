#ifndef ISO9660_HPP
#define ISO9660_HPP

#include <kernel/drivers/storage/FS/VFS.hpp>
#include <kernel/drivers/storage/FS/models/ISO9660/ISO9660.hpp>
#include <kernel/klibc/stdio>

class ISO9660 : public VFS {
private:
	ISO9660_model model;

public:
	ISO9660() {}
	ISO9660(uint32_t driveid) : model(driveid) {}

	inline bool isReadOnly() const override { return true; }
	inline bool isRAM() const override { return false; }

	FSRawChunk readFile(const string& path) override;
	void readFileTo(const string& path, FSRawChunk& theto) override;

	void newdir(const string& path) override {
		if(path == "") {}
		printf(" {{ jlxip's fault: newdir called on ISO9660 }} ");
		while(true) {}
	};

	set<string> ls(const string& path) override;

	bool isSymLink(const string& path, string& dst) override {
		// Filesystem does not support symlinks.
		if(path == "" || dst == "") {}
		return false;
	}

	inline void link(const string& orig, const string& dst) override {
		// Same as above.
		if(orig == "" || dst == "") {}
		printf(" {{ jlxip's fault: link called on ISO9660 }} ");
		while(true) {}
	}

	inline bool exists(const string& path) {
		ISO9660_model::entity entity = model.get(path.split('/'));
		return entity.found;
	}

	inline bool isDir(const string& path) override {
		// TODO: How do I do this? 🤔
		return exists(path);
	}

	inline bool isFile(const string& path) override {
		// TODO: How do I do this?
		return exists(path);
	}

	inline void newfile(const string& path, FSRawChunk contents) override {
		if(path == "" && contents.good()) {}
	}
};

#endif
