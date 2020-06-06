#ifndef VFS_HPP
#define VFS_HPP

#include <kernel/klibc/STL/string>
#include <kernel/klibc/STL/set>
#include <kernel/drivers/storage/FSRawChunk.hpp>

/*
	Layer of abstraction around filesystems.
*/
class VFS {
public:
	virtual bool isReadOnly() const = 0;
	virtual bool isRAM() const = 0;

	virtual FSRawChunk readFile(const string& path) = 0;
	virtual void readFileTo(const string& path, FSRawChunk& theto) = 0;
	virtual void newdir(const string& path) = 0;
	virtual set<string> ls(const string& path) = 0;
	virtual bool isSymLink(const string& path, string& dst) = 0;
	virtual void link(const string& orig, const string& dst) = 0;
	virtual bool isDir(const string& path) = 0;
	virtual bool isFile(const string& path) = 0;
	virtual void newfile(const string& path, FSRawChunk contents) = 0;
};

#endif
