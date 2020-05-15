#ifndef ISO9660_H
#define ISO9660_H

#define ISO9660_ROOT_RECORD_OFFSET 156
#define ISO9660_DIR_RECORD_LENGTH 0
#define ISO9660_DIR_EAR_LBA 2
#define ISO9660_DIR_EAR_LENGTH 10
#define ISO9660_DIR_FILENAME_LENGTH 32
#define ISO9660_DIR_FILENAME 33

#include <common/types.hpp>
#include <kernel/klibc/STL/list>
#include <kernel/klibc/STL/string>
#include <kernel/klibc/STL/set>
#include <kernel/klibc/STL/map>

// It can be either a directory record or a file itself.
struct ISO9660_entity {
	uint32_t LBA;
	uint32_t length;
};

// TODO: PLEASE GET RID OF THESE POINTERS!!!!!!!!!!!!!!!!
ISO9660_entity* ISO9660_get(const list<string>& dirs);

map<string, pair<uint32_t, uint32_t>> ISO9660_getChildren(uint32_t lba, uint32_t len);
set<string> ISO9660_getChildren_IamNotInsane(ISO9660_entity* entity);

// Why the fuck would this reserve memory? Omg this needs deep changes.
uint8_t* ISO9660_read(ISO9660_entity* entity);

// This is disgusting.
uint8_t* ISO9660_granularread(ISO9660_entity* entity);

#endif
