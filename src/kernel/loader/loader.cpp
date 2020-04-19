#include <kernel/loader/loader.hpp>
#include <kernel/loader/ELF/ELF.hpp>
#include <kernel/paging/paging.hpp>
#include <kernel/klibc/stdlib.hpp>

// Remove when implementing this for real.
#include <kernel/klibc/stdio>

// This does almost no checks and it's trivial to break.
list<uint8_t*> loadELF(uint8_t* data) {
	list<uint8_t*> pages;

	list<ELF::segment> segments = ELF::parse(data);
	for(auto const& x : segments) {
		// Will do this at some point. By now I just want to finish the parser.
		printf("Off(0x%x) Size(%d) to Addr(0x%x) Size(%d)\n", x.file_offset, x.file_length, x.start_addr, x.size_memory);

		/*
		OLD CODE IN CASE I WANT TO CHECK SOMETHING OFF IT.

		// Copy p_filesz bytes from p_offset to new pages.
		uint32_t npages = phe->p_filesz / PAGE_SIZE;
		for(uint32_t i=0; i<npages; ++i) {
			// Allocate the page.
			uint32_t page = paging_allocPages(1);

			// Set it to user mode.
			paging_setUser(page, 1);

			// By now, all pages are RWX. Don't care about that right now.
			// They should be changed here.

			// Copy the contents.
			memcpy((uint8_t*)page, data + phe->p_offset, PAGE_SIZE);

			pages.push_back((uint8_t*)page);
		}*/
	}

	// Create the process and probably call the scheduler.

	return pages;
}
