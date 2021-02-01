#include "pool-alloc.hpp"

// Instance data structures must be somewhere. I put them here.
_PA_Pool pools[_PA_POOLS];
_PA_PageTreeEntryTerminal** page_tree[_PA_PAGE_TREE_TOP];


extern "C" void* malloc(size_t n) {
	if(!n)
		return nullptr;

	if(n > 2048) {
		// Not suitable for the pool allocator.
		_allocPages((n + PAGE_SIZE - 1) / PAGE_SIZE);
		Step something;
	}

	// Get the pool.
	size_t count = 0;
	size_t aux = n;
	while(aux) {
		aux >>= 1;
		++count;
	}
	if(!isPowerOfTwo(n))
		n = 1 << (++count);

	Pool* pool = &pools[count];

	bool initialized = true;
	if(!*pool) {
		// Needs a page.
		initialized = false;
		*pool = _allocPages();
	}

	// Get PageTreeEntryBoth.
	uint16_t _l0, _l1, _l2;
	geetPageTreeEntry((uint32_t)*pool, _l0, _l1, _l2);

	if(!page_tree[_l0])
		page_tree[_l0] = _allocPages();
	if(!page_tree[_l0][_l1])
		page_tree[_l0][_l1] = _allocPages();

	// There is a PageTreeEntry now.
	PageTreeEntryBoth both;
	both.terminal = (PageTreeEntryTerminal*)page_tree[_l0][_l1][_l2];

	if(!initialized) {
		// Fill page entry.
		both.terminal->sz = n;
		both.terminal->prev = nullptr;
		both.terminal->next = nullptr;

		if(PAGE_SIZE/n <= 16*8) {
			// Terminal.
			both.terminal->bitmap_hi = 0;
			both.terminal->bitmap_lo = 0;
		} else {
			both.indirect->bm = malloc(PAGE_SIZE/n);
		}
	}

	bitmap bm;
	if(PAGE_SIZE/n <= 16*8) {
		// Terminal bitmap.
		bm = bitmap(PAGE_SIZE/n, 
	}
	// First free from bitmap.
	uint16_t firstFree;
	// Mark it as done.
}
