#ifndef ELF_H
#define ELF_H

#include <common/types.hpp>
#include <kernel/klibc/STL/list>

/*
	ELF specs taken from here:
	https://wiki.osdev.org/ELF
*/

#define ELF_PHEADER_SIZE 32

namespace ELF {
	// These are stuff for the loader.
	struct segment {
		uint32_t file_offset;
		uint32_t file_length;
		uint32_t start_addr;
		uint32_t size_memory;
		//uint32_t protFlags;	// At some point.
	};

	list<segment> parse(uint8_t* data);

	// These are real ELF types.
	enum class ELFtype : uint16_t {
		__NONE,
		RELOCATABLE,
		EXECUTABLE,
		SHARED,
		CORE
	};

	struct header {
		char magic[4];
		uint8_t bits;	// 1 (32) or 2 (64)
		uint8_t endianess;	// 1 (little) or 2 (big)
		uint8_t header_version;
		uint8_t OS_ABI;
		uint8_t padding[8];
		ELFtype elftype;
		uint16_t ISA;	// 3 for x86.
		uint32_t version;
		uint32_t p_entry;
		uint32_t p_header;
		uint32_t s_header;
		uint32_t flags;	// Unused in x86.
		uint16_t header_size;
		uint16_t size_entry_pheader;
		uint16_t n_entries_pheader;
		uint16_t size_entry_sheader;
		uint16_t n_entries_sheader;
		uint16_t index_names;
	};


	enum class segment_type : uint32_t {
		IGNORE,
		LOAD,
		DYNAMIC,
		INTERP,
		NOTE
	};

	struct segment_flags {
		enum {
			JUST_IGNORE_THIS,
			EXECUTABLE,
			WRITABLE,
			READABLE
		};
	};

	struct program_header_entry {
		segment_type stype;
		uint32_t p_offset;
		uint32_t p_vaddr;
		uint8_t undefined[4];
		uint32_t p_filesz;
		uint32_t p_memsz;
		uint32_t flags;
		uint32_t alignment;
	};
};

#endif
