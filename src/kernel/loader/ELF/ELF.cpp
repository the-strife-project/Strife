#include <kernel/loader/ELF/ELF.hpp>
#include <kernel/klibc/stdio>

void parseSegments(ELF::ParsedELF& elf, uint8_t* data, ELF::header* header) {
	for(uint16_t i=0; i<header->n_entries_pheader; ++i) {
		ELF::program_header_entry* phe = (ELF::program_header_entry*)(data + header->p_header + i*ELF_PHEADER_SIZE);

		// Check that the segment has to be loaded.
		if(phe->stype != ELF::segment_type::LOAD)
			continue;

		ELF::segment toAdd;
		toAdd.file_offset = phe->p_offset;
		toAdd.file_length = phe->p_filesz;
		toAdd.start_addr = phe->p_vaddr;
		toAdd.size_memory = phe->p_memsz;
		// TODO: ELF flags here.

		elf.segments.push_back(toAdd);
	}
}

char* get_shstrtab(uint8_t* data, ELF::header* header) {
	uint16_t shstrtab_idx = header->index_names;
	ELF::section_header_entry* shstrtab_entry = (ELF::section_header_entry*)(data + header->s_header + shstrtab_idx*ELF_SHEADER_SIZE);
	return (char*)(data + shstrtab_entry->offset);
}

ELF::sections_t getSections(uint8_t* data, ELF::header* header) {
	ELF::sections_t ret;

	char* shstrtab = get_shstrtab(data, header);
	for(uint16_t i=0; i<header->n_entries_sheader; ++i) {
		ELF::section_header_entry* she = (ELF::section_header_entry*)(data + header->s_header + i*ELF_SHEADER_SIZE);
		ret[shstrtab + she->name] = she;
	}

	return ret;
}

ELF::ParsedELF ELF::parse(uint8_t* data, bool isLibrary) {
	ELF::ParsedELF ret;

	ELF::header* header = (ELF::header*)data;
	ret.sections = getSections(data, header);

	parseSegments(ret, data, header);

	// Look for ".dynstr" and ".dynamic".
	char* dynstr = nullptr;

	auto* aux = ret.sections[".dynstr"];
	if(aux)
		dynstr = (char*)(data + aux->offset);
	auto* dynamic = ret.sections[".dynamic"];

	if(dynamic && dynstr) {
		// There is a dynamic section. Time to look for dependencies.
		for(uint32_t off=0; off<dynamic->size; off+=dynamic->entsize) {
			ELF::dynamic_entry* dent = (ELF::dynamic_entry*)(data + dynamic->offset + off);
			if(dent->needed == 1)
				ret.libraries.push_back(string(dynstr + dent->value));
		}
	}

	auto* dynsym = ret.sections[".dynsym"];

	if(isLibrary && dynstr && dynsym) {
		// Start looking for global symbols.
		for(uint32_t off=0; off<dynsym->size; off+=dynsym->entsize) {
			ELF::dynsym_entry* dsent = (ELF::dynsym_entry*)(data + dynsym->offset + off);
			if(dsent->info == ELF_ST_INFO_GLOBAL_FUNC)
				ret.globalFunctions[string(dynstr + dsent->name)] = dsent->value;
		}
	}

	// Get dynamic references.
	auto* reldyn = ret.sections[".rel.dyn"];
	if(reldyn && dynsym && dynstr) {
		for(uint32_t off=0; off<reldyn->size; off+=reldyn->entsize) {
			rel_entry* relent = (rel_entry*)(data + reldyn->offset + off);

			if((relent->info & 0xFF) != (uint8_t)relocation_type::R_386_JMP_SLOT) {
				// TODO: Fail less miserably.
				printf(" {{ jlxip's fault: unsupported relocation type. }} ");
				while(true) {}
			}

			uint8_t symbol_idx = relent->info >> 8;
			dynsym_entry* dynsyment = (dynsym_entry*)(data + dynsym->offset + dynsym->entsize*symbol_idx);
			ret.dynReferences[dynstr + dynsyment->name] = (uint32_t*)(relent->offset);
		}
	}

	return ret;
}
