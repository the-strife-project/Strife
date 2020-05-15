#include <kernel/drivers/storage/FS/models/ISO9660/ISO9660.hpp>
#include <kernel/drivers/storage/ATAPI_PIO/ATAPI_PIO.hpp>
#include <kernel/klibc/stdlib.hpp>

ISO9660_entity* ISO9660_get(const list<string>& dirs) {
	// Get the root directory record extent as 'last'.
	ATAPI_read(1, 0x10);
	uint32_t last_len = *(uint32_t*)(
		ATAPI_PIO_BUFFER +
		ISO9660_ROOT_RECORD_OFFSET +
		ISO9660_DIR_EAR_LENGTH
	);
	uint32_t last_LBA = *(uint32_t*)(
		ATAPI_PIO_BUFFER +
		ISO9660_ROOT_RECORD_OFFSET +
		ISO9660_DIR_EAR_LBA
	);

	// Run through 'dirs'.
	for(auto const& x : dirs) {
		if(x == "")
			continue;

		map<string, pair<uint32_t, uint32_t>> children(ISO9660_getChildren(last_LBA, last_len));

		bool found = false;
		for(auto const& y : children) {
			if(y.f == x) {
				// Found it.
				found = true;
				last_LBA = y.s.f;
				last_len = y.s.s;
				break;
			}
		}

		// Seems like there's no such directory.
		if(!found)
			return nullptr;
	}

	ISO9660_entity* ret = new ISO9660_entity;
	ret->LBA = last_LBA;
	ret->length = last_len;
	return ret;
}

map<string, pair<uint32_t, uint32_t>> ISO9660_getChildren(uint32_t lba, uint32_t len) {
	map<string, pair<uint32_t, uint32_t>> ret;

	ATAPI_read((len % ATAPI_SECTOR_SIZE != 0) + (len / ATAPI_SECTOR_SIZE), lba);

	uint32_t ctr = 0;
	uint32_t i = 0;
	while(i < len) {
		// Check if the record length is ok.
		uint32_t increment = *(uint8_t*)(ATAPI_PIO_BUFFER + i + ISO9660_DIR_RECORD_LENGTH);
		if(!increment)
			break;

		// The first two ("." and "..")?
		if(ctr++ < 2) {
			// We don't care about those.
			i += increment;
			continue;
		}

		// Get the filename.
		char* filename = (char*)(ATAPI_PIO_BUFFER + i + ISO9660_DIR_FILENAME);
		for(uint32_t j=0; j<ISO9660_DIR_FILENAME_LENGTH; ++j) {
			if(filename[j] == ';') {
				filename[j] = 0;
				break;
			}
		}

		uint32_t this_lba = *(uint32_t*)(ATAPI_PIO_BUFFER + i + ISO9660_DIR_EAR_LBA);
		uint32_t this_len = *(uint32_t*)(ATAPI_PIO_BUFFER + i + ISO9660_DIR_EAR_LENGTH);

		ret[filename] = pair<uint32_t, uint32_t>(this_lba, this_len);
		i += increment;
	}

	return ret;
}

set<string> ISO9660_getChildren_IamNotInsane(ISO9660_entity* entity) {
	map<string, pair<uint32_t, uint32_t>> children(ISO9660_getChildren(entity->LBA, entity->length));

	set<string> ret;
	for(auto const& x : children)
		ret.insert(x.f);
	return ret;
}

uint8_t* ISO9660_read(struct ISO9660_entity* entity) {
	ATAPI_read(
		(entity->length % 2048 != 0) + (entity->length / 2048),
		entity->LBA
	);
	return (uint8_t*)ATAPI_PIO_BUFFER;
}

uint8_t* ISO9660_granularread(ISO9660_entity* entity) {
	uint8_t* ret = new uint8_t[entity->length + ATAPI_SECTOR_SIZE];
	ATAPI_granularread(1+(entity->length / ATAPI_SECTOR_SIZE), entity->LBA, ret);
	return ret;
}
