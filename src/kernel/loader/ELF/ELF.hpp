#ifndef ELF_H
#define ELF_H

#include <common/types.hpp>
#include <kernel/klibc/STL/list>
#include <kernel/klibc/STL/string>
#include <kernel/klibc/STL/map>

/*
	ELF specs taken from here:
	https://wiki.osdev.org/ELF
*/

#define ELF_PHEADER_SIZE 32
#define ELF_SHEADER_SIZE 40
#define ELF_ST_INFO_GLOBAL_FUNC 0x12

namespace ELF {
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

	// -- Sections --
	enum class section_type : uint32_t {
		SNULL,
		PROGBITS,
		SYMTAB,
		STRTAB,
		RELA,
		HASH,
		DYNAMIC,
		NOTE,
		NOBITS,
		REL,
		SHLIB,
		DYNSYM,
		INIT_ARRAY,
		FINI_ARRAY,
		PREINIT_ARRAY,
		GROUP,
		SYMTAB_SHNDX,
		NUM
	};

	struct section_header_entry {
		uint32_t name;	// Offset to string in .shstrtab
		section_type type;
		uint32_t flags;
		uint32_t addr;
		uint32_t offset;
		uint32_t size;
		uint32_t link;
		uint32_t info;
		uint32_t addralign;
		uint32_t entsize;
	};

	// -- Dynamic --
	struct dynamic_entry {
		uint32_t needed;
		uint32_t value;
	};

	struct dynsym_entry {
		uint32_t name;
		uint32_t value;
		uint32_t size;
		uint8_t info;
		uint8_t other;
		uint16_t shndx;
	};

	struct rel_entry {
		uint32_t offset;
		uint32_t info;
	};

	enum class relocation_type : uint8_t {
		R_386_NONE,
		R_386_32,
		R_386_PC32,
		R_386_GOT32,
		R_386_PLT32,
		R_386_COPY,
		R_386_GLOB_DAT,
		R_386_JMP_SLOT,
		R_386_RELATIVE,
		R_386_GOTOFF,
		R_386_GOTPC,
		R_386_32PLT
	};



	// These are stuff for the loader.
	struct segment {
		uint32_t file_offset;
		uint32_t file_length;
		uint32_t start_addr;
		uint32_t size_memory;
		//uint32_t protFlags;	// At some point.
	};

	typedef map<string, ELF::section_header_entry*> sections_t;

	struct ParsedELF {
		list<segment> segments;
		sections_t sections;
		list<string> libraries;
		map<string, uint32_t> globalFunctions;	// (name, offset)
		map<string, uint32_t*> dynReferences;
	};

	ParsedELF parse(uint8_t* data);
};

#endif
