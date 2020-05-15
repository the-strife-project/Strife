#ifndef VFS_HPP
#define VFS_HPP

#include <kernel/klibc/STL/string>
#include <kernel/klibc/STL/set>

/*
	Basically a high level pointer.
	Believe me, this makes a difference; it makes
	the code so much easier to read (and write).
*/
class FSRawChunk {
private:
	uint8_t* data;
	uint64_t size;

public:
	// Default is not "good()".
	FSRawChunk() : data(nullptr), size(0) {}

	FSRawChunk(uint64_t size) : size(size) {
		data = new uint8_t[size];
	}

	FSRawChunk(uint8_t* data, uint64_t size) : data(data), size(size) {}

	inline uint8_t* get() {
		return data;
	}

	inline string str() const {
		string ret;
		for(uint64_t i=0; i<size; ++i)
			ret += (char)data[i];
		return ret;
	}

	inline uint64_t getSize() const {
		return size;
	}

	inline bool good() const {
		return (size != 0);
	}

	inline void destroy() {
		if(size) {
			delete [] data;
			data = nullptr;
			size = 0;
		}
	}

	inline void invalidate() {
		size = 0;
	}
};

/*
	Layer of abstraction around filesystems.
*/
class VFS {
public:
	virtual bool isReadOnly() const = 0;
	virtual bool isRAM() const = 0;

	// TODO: THIS BOOL IS USELESS ONCE I GET DMA WORKING.
	virtual FSRawChunk readFile(const string& path, bool big=false) = 0;
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
