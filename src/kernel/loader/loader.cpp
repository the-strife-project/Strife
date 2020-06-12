#include <kernel/loader/loader.hpp>
#include <kernel/paging/paging.hpp>
#include <kernel/klibc/stdlib.hpp>
#include <kernel/klibc/stdio>
#include <kernel/mounts/mounts.hpp>

map<string, SharedLibrary*> loadedSharedLibraries;
bool mapInitialized = false;

bool Program::loadDynamicLibraries() {
	if(!mapInitialized) {
		loadedSharedLibraries = map<string, SharedLibrary*>();
		mapInitialized = true;
	}

	for(auto const& x : elf.libraries) {
		// Already loaded?
		if(!loadedSharedLibraries[x]) {
			// Must load now.
			FSRawChunk lib_raw = readFile(string("/lib/") + x);
			if(!lib_raw.good()) {
				failedDynamicLibrary = x;
				return false;
			}

			SharedLibrary* lib = new SharedLibrary;
			lib->parse(lib_raw.get());
			lib->load();

			lib_raw.destroy();
			loadedSharedLibraries[x] = lib;
		}

		SharedLibrary* thisLibrary = loadedSharedLibraries[x];
		for(auto const& x : thisLibrary->getGlobalFunctions())
			dynFunctions[x.f] = FunctionPair(thisLibrary, x.s);
	}

	return true;
}

void ELFSomething::load() {
	// TODO: Segments in the GDT.
	map<uint32_t, uint32_t> alreadyAllocated;	// (virt, phys)
	for(auto const& x : elf.sections) {
		if(x.s->addr == 0)
			continue;

		uint32_t npages = x.s->size / PAGE_SIZE;
		if(x.s->size % PAGE_SIZE)
			++npages;

		for(uint32_t i=0; i<npages; ++i) {
			uint32_t virtPage = (x.s->addr + i*PAGE_SIZE) & ~0xFFF;
			if(alreadyAllocated.find(pair<uint32_t, uint32_t>(virtPage, 0)) == alreadyAllocated.end()) {
				uint32_t page = paging_allocPages(1);
				paging_setUser(page);
				alreadyAllocated[virtPage] = page;
				pages.push_back(pair<uint32_t, uint32_t>(alreadyAllocated[virtPage], x.s->addr + PAGE_SIZE*i));
			}

			// Permissions?

			memcpy(
				(uint8_t*)alreadyAllocated[virtPage] + ((x.s->offset - i*PAGE_SIZE) % PAGE_SIZE),
				raw + x.s->offset,
				(x.s->size - i*PAGE_SIZE) % PAGE_SIZE);
		}
	}

	/*
		Mount shared library on pages.
		Virtual pages will be offsetted with the first physical page.
	*/
	for(auto const& x : elf.libraries) {
		SharedLibrary* lib = loadedSharedLibraries[x];
		uint32_t firstPhysicalPage = (*(lib->getPages().begin())).f;
		for(auto const& page : lib->getPages())
			pages.push_back(pair<uint32_t, uint32_t>(page.f, firstPhysicalPage + page.s));
		libraryMounts[lib] = firstPhysicalPage;
	}
}

bool ELFSomething::relocate() {
	for(auto const& x : elf.dynReferences) {
		auto func = dynFunctions[x.f];

		if(!func.f) {
			failedRelocation = x.f;
			return false;
		}

		// Fill the got entry.
		*(x.s) = libraryMounts[func.f] + func.s;
	}

	// TODO: make GOT pages RO.
	return true;
}
