#ifndef ISO9660_H
#define ISO9660_H

#define ISO9660_ROOT_RECORD_OFFSET 156
#define ISO9660_DIR_RECORD_LENGTH 0
#define ISO9660_DIR_EAR_LBA 2
#define ISO9660_DIR_EAR_LENGTH 10
#define ISO9660_DIR_FILENAME_LENGTH 32
#define ISO9660_DIR_FILENAME 33

#include <common/types.hpp>
#include <list>
#include <string>
#include <set>
#include <map>
#include <kernel/drivers/storage/FSRawChunk.hpp>

class ISO9660_model {
public:
	// It can be either a directory record or a file itself.
	struct entity {
		uint32_t LBA;
		uint32_t length;
		bool found;
	};

private:
	uint8_t driveid;

public:
	ISO9660_model();
	ISO9660_model(uint8_t driveid);

	entity get(const list<string>& dirs);
	map<string, pair<uint32_t, uint32_t>> getChildren(uint32_t lba, uint32_t len);
	set<string> getChildren_IamNotInsane(const entity& entity);
	FSRawChunk read(const entity& entity);
};

#endif
