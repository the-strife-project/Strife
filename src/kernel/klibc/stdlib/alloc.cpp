#include <kernel/paging/paging.hpp>

extern "C" void* _allocPages(size_t n) {
	return kernelPaging.alloc(n);
}

extern "C" void* _freePage(size_t page) {
	return kernelPaging.free(page);
}
