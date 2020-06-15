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

/*
	Bear with me.

	load() generates "pages", a list of PhysVirt (and fills the pages).
	"pages" may contain the pages of the binary as well as those of
	the needed shared libraries.
	PhysVirt is not composed of (physical page, virtual page), but
	(physical page, virtual offset).
	This way, "pages", when load() is finished, contain offsets.
	relocate() is the function that turns these offsets into actual
	virtual addresses.

	This all works via separators in "pages", that is, PhysVirt objects
	that contain phys=0.

	I don't know how other operating systems do this, probably easier,
	but this is what I came up with.
*/

void ELFSomething::load() {
	map<uint32_t, uint32_t> alreadyAllocated;	// (virt, phys)
	for(auto const& x : elf.sections) {
		// Load only sections with ALLOC flag.
		if(!(x.s) || !(x.s->flags & ELF_SHF_ALLOC))
			continue;

		uint32_t npages = x.s->size / PAGE_SIZE;
		if(x.s->size % PAGE_SIZE)
			++npages;

		for(uint32_t i=0; i<npages; ++i) {
			uint32_t virtPage = (x.s->addr + i*PAGE_SIZE) & ~0xFFF;

			// If current page is not allocated, do it now.
			if(alreadyAllocated.find(pair<uint32_t, uint32_t>(virtPage, 0)) == alreadyAllocated.end()) {
				uint32_t page = paging_allocPages(1);
				alreadyAllocated[virtPage] = page;

				pages.push_back(PhysVirt(alreadyAllocated[virtPage], virtPage));
			}

			// Copy contents.
			uint32_t sizeSubPage = (x.s->size - i*PAGE_SIZE) % PAGE_SIZE;
			if(!sizeSubPage)
				sizeSubPage = PAGE_SIZE;

			memcpy((uint8_t*)alreadyAllocated[virtPage] + ((x.s->addr - i*PAGE_SIZE) % PAGE_SIZE),
				   raw + x.s->offset,
				   sizeSubPage);
		}
	}

	// Mount shared libraries.
	for(auto const& x : elf.libraries) {
		pages.push_back(PhysVirt(0, 0));	// Separator.

		SharedLibrary* lib = loadedSharedLibraries[x];
		for(auto const& page : lib->getPages())
			pages.push_back(PhysVirt(page.phys, page.virtoff));
	}

	pages.push_back(PhysVirt(0, 0));
}

void ELFSomething::relocate() {
	/*
		This variable is a substitute for ASLR whilst I don't have it.
		Libraries and sections will be loaded starting from this page
		upwards (to 0xFFFFFFFF).
	*/
	const uint32_t pageRealBeginning = 0x8CACA000;	// Real beginning.

	uint32_t pageBeginning = pageRealBeginning;	// Beginning for this "thing".
	beginning = pageRealBeginning;

	list<PhysVirt> newpages;	// New pages (no virtoff, only phys and virt).
	uint32_t max = pageBeginning;

	bool binaryDone = false;
	auto lib_it = elf.libraries.begin();

	for(auto const& x : pages) {
		if(x.phys == 0) {
			// Separator.
			if(binaryDone) {
				// Just finished a library. Time to update where it's mounted.
				libraryMounts[loadedSharedLibraries[*lib_it]] = pageBeginning;
				++lib_it;
			} else {
				// Just finished the binary. Next will be a library.
				binaryDone = true;
			}

			if(lib_it == elf.libraries.end())
				break;

			// Update the beginning page of the current "thing" (library or binary).
			pageBeginning = (max + PAGE_SIZE) & ~0xFFF;
			continue;
		}

		PhysVirt pv;
		pv.phys = x.phys;
		pv.virtoff = x.virtoff;	// Debug. Not really needed.
		pv.virt = x.virtoff + pageBeginning;
		newpages.push_back(pv);

		if(max < pv.virt)
			max = pv.virt;
	}

	pages = newpages;
}

// Must be called once the new pages are set.
bool ELFSomething::relocate2() {
	for(auto const& x : elf.dynReferences) {
		auto func = dynFunctions[x.f];

		if(!func.f) {
			failedRelocation = x.f;
			return false;
		}

		// Fill the got entry.
		// This only works with the binary. No relocation for libraries. Therefore, no nested libraries (TODO).
		uint32_t* gotent = (uint32_t*)(beginning + (uint32_t)x.s);
		*gotent = libraryMounts[func.f] + func.s;
	}

	// RELRO here.

	return true;
}
