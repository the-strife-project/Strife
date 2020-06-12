#ifndef LOADER_H
#define LOADER_H

#include <common/types.hpp>
#include <kernel/loader/ELF/ELF.hpp>
#include <kernel/klibc/STL/list>
#include <kernel/klibc/STL/pair>
#include <kernel/klibc/STL/map>
#include <kernel/klibc/STL/string>

class SharedLibrary;

// I just want this to make sure SharedLibrary* is initialized to nullptr.
struct FunctionPair : public pair<SharedLibrary*, uint32_t> {
	FunctionPair() {
		this->f = nullptr;
	}
	FunctionPair(SharedLibrary* t, uint32_t q) {
		this->f = t;
		this->s = q;
	};
};

class ELFSomething {
protected:
	uint8_t* raw;
	ELF::ParsedELF elf;
	map<string, FunctionPair> dynFunctions;	// (name, (library, offset in library))
	list<pair<uint32_t, uint32_t>> pages;	// (phys, virt)
	map<SharedLibrary*, uint32_t> libraryMounts;
	string failedRelocation;

public:
	inline void parse(uint8_t* _raw) {
		raw = _raw;
		elf = ELF::parse(raw);
	}

	void load();
	bool relocate();

	inline const list<pair<uint32_t, uint32_t>>& getPages() const {
		return pages;
	}

	inline const string& getFailedRelocation() const {
		return failedRelocation;
	}
};


class SharedLibrary : public ELFSomething {
public:
	// TODO: Why can't I make this const?
	inline map<string, uint32_t>& getGlobalFunctions() {
		return elf.globalFunctions;
	}

	// TODO: nested shared libraries.
};

class Program : public ELFSomething {
private:
	string failedDynamicLibrary;

public:
	bool loadDynamicLibraries();

	inline string getFailedDynamicLibrary() const {
		return failedDynamicLibrary;
	}
};

#endif
