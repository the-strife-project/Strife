#include "pool-alloc.hpp"
#include <bitmap>

bool _bitSet(PageTreeEntryTerminal* term, size_t bit, bool value) {
	uint8_t* bm;

	if(term->size <= _PA_MAX_TERMINAL_BITMAP_SZ) {
		// Terminal bitmap.
		bm = &(term->bitmap_hi);
	} else {
		// Indirect bitmap.
		PageTreeEntryBoth both;
		both.terminal = term;
		bm = (uint8_t*)(both.indirect->bm);
	}

	bitmap b(bm, 4096 / term->sz);
	b.set(bit, value);

	if(value) {
		for(size_t i=0; i<((term->sz + 7)/8); ++i)
			if(*(bm++) != ~0)
				return false;
	} else {
		for(size_t i=0; i<((term->sz + 7)/8); ++i)
			if(*(bm++) != 0)
				return false;
	}

	return true;
}
