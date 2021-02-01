#ifndef LOADER_H
#define LOADER_H

#include <common/types.hpp>
#include <kernel/loader/ELF/ELF.hpp>
#include <list>
#include <pair>
#include <map>
#include <string>

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

struct PhysVirt {
	uint32_t phys;
	uint32_t virtoff;
	uint32_t virt;	// virt = beginning + virtoff.
	bool ro, cow, exe;

	PhysVirt()
		: phys(0), virtoff(0), virt(0), ro(false), cow(false), exe(true)
	{}

	PhysVirt(uint32_t phys, uint32_t virtoff)
		: phys(phys), virtoff(virtoff), virt(0), ro(false), cow(false), exe(true)	// TODO COW
	{}
};

class ELFSomething {
protected:
	uint8_t* raw;
	ELF::ParsedELF elf;
	map<string, FunctionPair> dynFunctions;	// (name, (library, offset in library))
	list<PhysVirt> pages;
	map<SharedLibrary*, uint32_t> libraryMounts;
	string failedRelocation;
	uint32_t beginning;	// The base virtual address.

public:
	void load();
	void relocate();
	bool relocate2();

	inline uint32_t getEntryPoint() const {
		return beginning + elf.entryPoint;
	}

	inline const list<PhysVirt>& getPages() const {
		return pages;
	}

	inline const string& getFailedRelocation() const {
		return failedRelocation;
	}
};


class SharedLibrary : public ELFSomething {
public:
	inline void parse(uint8_t* _raw) {
		raw = _raw;
		elf = ELF::parse(raw, true);	// true: shared object.
	}

	// TODO: Why can't I make this const?
	inline map<string, uint32_t>& getGlobalFunctions() {
		return elf.globalFunctions;
	}
};

class Program : public ELFSomething {
private:
	string failedDynamicLibrary;

public:
	inline void parse(uint8_t* _raw) {
		raw = _raw;
		elf = ELF::parse(raw, false);	// false: executable.
	}

	bool loadDynamicLibraries();

	inline string getFailedDynamicLibrary() const {
		return failedDynamicLibrary;
	}
};

#endif
