#include <kernel/loader/ELF/ELF.hpp>

list<ELF::segment> ELF::parse(uint8_t* data) {
	list<ELF::segment> ret;

	ELF::header* header = (ELF::header*)data;

	for(uint16_t i=0; i<header->n_entries_pheader; ++i) {
		// Each entry is 32 bytes.
		ELF::program_header_entry* phe = (ELF::program_header_entry*)(data + header->p_header + i*ELF_PHEADER_SIZE);

		// Check that the segment has to be loaded.
		if(phe->stype != ELF::segment_type::LOAD) continue;

		ELF::segment toAdd;
		toAdd.file_offset = phe->p_offset;
		toAdd.file_length = phe->p_filesz;
		toAdd.start_addr = phe->p_vaddr;
		toAdd.size_memory = phe->p_memsz;
		// Flags here. When I do it, that is.

		ret.push_back(toAdd);
	}

	return ret;
}
