#ifndef _PA_HPP
#define _PA_HPP

#include <types.hpp>

#define _PA_POOLS 12
#define _PA_PAGE_TREE_TOP 8
#define _PA_PAGE_TREE_MID 1024
#define _PA_PAGE_TREE_BOT 146
#define _PA_MAX_POOL_SZ 2048
#define _PA_MAX_TERMINAL_BITMAP_SZ 16

// This function must be given externally.
extern "C" void* _allocPages(size_t n=1);
extern "C" void* _freePage(size_t page);

// A pool is just defined by its first non-empty, non-full page.
typedef void* _PA_Pool;

struct _PA_PageTreeEntryTerminal {
	uint32_t sz;
	uint64_t bitmap_hi;
	uint64_t bitmap_lo;
	PageTreeEntryTerminal* prev;
	PageTreeEntryTerminal* next;
};

struct _PA_PageTreeEntryIndirect {
	uint32_t sz;
	void* bm;
	uint32_t reserved0;
	uint64_t reserved1;
	PageTreeEntryIndirect* prev;
	PageTreeEntryIndirect* next;
};

union _PA_PageTreeEntryBoth {
	PageTreeEntryTerminal* terminal;
	PageTreeEntryIndirect* indirect;
};

inline constexpr bool isPowerOfTwo(size_t n) {
	return (n & (n - 1)) == 0;
}

// Instance data structures.
extern _PA_Pool* pools;
extern _PA_PageTreeEntryTerminal*** page_tree;

void* _PA_getFreePage();
void _PA_freePage(size_t page);
uint16_t _PA_getPageTreeIdx(uint32_t page, uint8_t height);
void _PA_getPageTreeEntry(uint32_t page, uint16_t& l0, uint16_t& l1, uint16_t& l2);
extern "C" void* malloc(uint32_t n);
extern "C" void free(void* ptr);

#endif
