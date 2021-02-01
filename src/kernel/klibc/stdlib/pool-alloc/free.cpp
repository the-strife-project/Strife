#include "pool-alloc.hpp"

extern "C" void free(void* ptr) {
	uint32_t page = ((uint32_t)ptr) & ~0xFFF;

	uint16_t _l0, _l1, _l2;
	geetPageTreeEntry(page, _l0, _l1, _l2);

	// Make sure it's allocated.
	if(!page_tree[_l0])
		return;
	if(!page_tree[_l0][_l1])
		return;

	PageTreeEntryTerminal* term = (PageTreeEntryTerminal*)&(page_tree[_l0][_l1][_l2]);

	if(!term->sz)
		return;

	if(term->sz > _PA_MAX_POOL_SZ) {
		// Not in a pool. Free the page to the OS.
		term->sz = 0;
		_freePage(page);
		return;
	}

	// Bit in bitmap?
	size_t bit = ((uint32_t)ptr) & 0xFFF;
	bit /= term->size;

	// Set that bit as free.
	bool allFree = _bitSet(term, bit);
	if(allFree) {
		// Time to free this page.
		if(term->prev)
			term->prev->next = term->next;
		if(term->next)
			term->next->prev = term->prev;

		// Geet pool.
		size_t aux = term->size;
		size_t count = 0;
		while(aux) {
			aux >>= 1;
			++count;
		}
		Pool* pool = &pools[count];

		if(*pool == (void*)page) {
			// Set as the first one, change to next.
			*pool = (void*)(term->next);
			// Add to free pages.
			_PA_freePage(page);
		}
	}
};
