#include <kernel/drivers/storage/FS/models/ISO9660/ISO9660.hpp>
#include <kernel/drivers/storage/IDE/IDE.hpp>
#include <kernel/klibc/stdlib.hpp>

ISO9660_model::ISO9660_model() : driveid(4) {}
ISO9660_model::ISO9660_model(uint8_t driveid) : driveid(driveid) {}

ISO9660_model::entity ISO9660_model::get(const list<string>& dirs) {
	// Get the root directory record extent as 'last'.
	FSRawChunk chunk = ide.ATAPI_read(driveid, 0x10, 1);	// Shouldn't that 0x10 be a constant?

	uint32_t last_len = *(uint32_t*)(
		chunk.get() +
		ISO9660_ROOT_RECORD_OFFSET +
		ISO9660_DIR_EAR_LENGTH
	);
	uint32_t last_LBA = *(uint32_t*)(
		chunk.get() +
		ISO9660_ROOT_RECORD_OFFSET +
		ISO9660_DIR_EAR_LBA
	);

	ISO9660_model::entity ret;
	ret.found = false;

	// Run through 'dirs'.
	for(auto const& x : dirs) {
		if(x == "")
			continue;

		map<string, pair<uint32_t, uint32_t>> children(getChildren(last_LBA, last_len));

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
			return ret;
	}

	ret.LBA = last_LBA;
	ret.length = last_len;
	ret.found = true;
	return ret;
}

map<string, pair<uint32_t, uint32_t>> ISO9660_model::getChildren(uint32_t lba, uint32_t len) {
	map<string, pair<uint32_t, uint32_t>> ret;

	FSRawChunk chunk = ide.ATAPI_read(driveid, lba, (len % ATAPI_SECTOR_SIZE != 0) + (len / ATAPI_SECTOR_SIZE));

	uint32_t ctr = 0;
	uint32_t i = 0;
	while(i < len) {
		// Check if the record length is ok.
		uint32_t increment = *(uint8_t*)(chunk.get() + i + ISO9660_DIR_RECORD_LENGTH);
		if(!increment)
			break;

		// The first two ("." and "..")?
		if(ctr++ < 2) {
			// We don't care about those.
			i += increment;
			continue;
		}

		// Get the filename.
		uint8_t filename_len = *(uint8_t*)(chunk.get() + i + ISO9660_DIR_FILENAME_LENGTH);

		string filename;
		char* filename_orig = (char*)(chunk.get() + i + ISO9660_DIR_FILENAME);

		for(uint32_t j=0; j<filename_len; ++j) {
			if(filename_orig[j] == ';')
				break;

			filename += filename_orig[j];
		}

		uint32_t this_lba = *(uint32_t*)(chunk.get() + i + ISO9660_DIR_EAR_LBA);
		uint32_t this_len = *(uint32_t*)(chunk.get() + i + ISO9660_DIR_EAR_LENGTH);

		ret[filename] = pair<uint32_t, uint32_t>(this_lba, this_len);
		i += increment;
	}

	return ret;
}

set<string> ISO9660_model::getChildren_IamNotInsane(const ISO9660_model::entity& entity) {
	map<string, pair<uint32_t, uint32_t>> children(getChildren(entity.LBA, entity.length));

	set<string> ret;
	for(auto const& x : children)
		ret.insert(x.f);
	return ret;
}

FSRawChunk ISO9660_model::read(const ISO9660_model::entity& entity) {
	return ide.ATAPI_read(driveid, entity.LBA, (entity.length % 2048 != 0) + (entity.length / 2048));
}
