#ifndef ISO9660_H
#define ISO9660_H

#define ISO9660_ROOT_RECORD_OFFSET 156
#define ISO9660_DIR_RECORD_LENGTH 0
#define ISO9660_DIR_EAR_LBA 2
#define ISO9660_DIR_EAR_LENGTH 10
#define ISO9660_DIR_FILENAME_LENGTH 32
#define ISO9660_DIR_FILENAME 33

#include <common/types.h>
#include <klibc/STL/list>
#include <klibc/string>

// It can be either a directory record or a file itself.
struct ISO9660_entity {
	uint32_t LBA;
	uint32_t length;
};

struct ISO9660_entity* ISO9660_get(const list<string>& dirs);
uint8_t* ISO9660_read(struct ISO9660_entity* entity);

#endif
