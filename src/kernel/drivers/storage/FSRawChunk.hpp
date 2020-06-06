#ifndef FSRAWCHUNK_HPP
#define FSRAWCHUNK_HPP

#include <kernel/klibc/stdlib.hpp>

/*
	Basically a high level pointer.
	Believe me, this makes a difference.
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
		for(uint64_t i=0; i<size && data[i]; ++i)
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

#endif
