#include "pool-alloc.hpp"

size_t free_top = nullptr;
void* _PA_getFreePage() {
	if(free_top) {
		// Pop.
		void* ret = (void*)free_top;
		size_t next = *(size_t*)free_top;
		free_top = (void*)next;
		return ret;
	}

	return _allocPages();
}

void _PA_freePage(size_t page) {
	if(free_top) {
		size_t* contents = (size_t*)page;
		*contents = free_top;
	}
	free_top = page;
}

uint16_t _PA_getPageTreeIdx(size_t page, uint8_t height) {
	uint32_t mod = _PA_PAGE_TREE_BOT;
	uint32_t denom = 1;

	if(height > 0) {
		mod *= _PA_PAGE_TREE_MID;
		denom *= _PA_PAGE_TREE_BOT;
	}

	if(height > 1) {
		mod *= _PA_PAGE_TREE_TOP;
		denom *= _PA_PAGE_TREE_MID;
	}

	return (page % mod) / denom;
}

void _PA_getPageTreeEntry(uint32_t page, uint16_t& l0, uint16_t& l1, uint16_t& l2) {
	l2 = getPageTreeIdx(page, 0);
	l1 = getPageTreeIdx(page, 1);
	l0 = getPageTreeIdx(page, 2);
}
