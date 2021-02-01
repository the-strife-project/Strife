#ifndef PAGING_H
#define PAGING_H

#include <common/types.hpp>
#include <kernel/klibc/stdlib.hpp>

#define PAGE_SIZE 4096

#define PD_PRESENT  (1 << 0)
#define PD_RW       (1 << 1)
#define PD_ALL_PRIV (1 << 2)
#define PD_WRITETHR (1 << 3)
#define PD_CACHE_D  (1 << 4)
#define PD_ACCESSED (1 << 5)
#define PD_4M_PAGE  (1 << 7)

#define PT_PRESENT  (1 << 0)
#define PT_RW       (1 << 1)
#define PT_ALL_PRIV (1 << 2)
#define PT_WRITETHR (1 << 3)
#define PT_CACHE_D  (1 << 4)
#define PT_ACCESSED (1 << 5)
#define PT_DIRTY    (1 << 6)
#define PT_GLOBAL   (1 << 8)

class Paging {
private:
	uint32_t* pageDirectory;

public:
	struct Page {
	private:
		// Things that will never have to be changed.
		size_t virt;
		uint16_t pdi, pti;	// Indexes (so there's no need to calculate them every time).
		uint32_t* pt;	// Page table containing this for the same reason.

	public:
		Paging* parent;
		size_t phys;

		bool present, user;
		bool ro, cow, exe;	// Read-only, Copy-on-write, executable.
		bool dnf;	// Do not free.

		Page();
		Page(Paging* parent, size_t phys, size_t virt);

		inline uint32_t getVirt() const {
			return virt;
		}

		inline uint32_t& getRaw() {
			return pt[pti];
		}

		void flush();
		void free();

		inline bool operator<(const Page& other) const {
			return virt < other.virt;
		}

		inline bool operator==(const Page& other) const {
			return (parent == other.parent) && (phys == other.phys) && (virt == other.virt);
		}
	};

	Paging() : pageDirectory(nullptr) {}
	Paging(uint32_t* pageDirectory) : pageDirectory(pageDirectory) {}

	void copyFrom(Paging other);

	uint32_t getPhys(uint32_t virt);

	inline Page get(uint32_t virt) {
		return Page(this, getPhys(virt), virt);
	}

	uint32_t findConsecutive(size_t count);
	uint32_t alloc(size_t count=1);

	inline uint32_t calloc(size_t count=1) {
		uint32_t ret = alloc(count);
		memclear((void*)ret, count * PAGE_SIZE);
		return ret;
	}

	void free(uint32_t virt, size_t count=1);

	inline void destroyAfterFree() {
		delete pageDirectory;
		pageDirectory = nullptr;
	}

	void use();
};

extern Paging kernelPaging;

void paging_init();

extern "C" void go_paging();
extern "C" void goback_paging();

#endif
