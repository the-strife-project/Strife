#include <kernel/drivers/storage/FS/ISO9660/ISO9660.h>
#include <kernel/drivers/storage/ATAPI_PIO/ATAPI_PIO.h>
#include <libc/stdlib.h>

#include <libc/stdio.h>
/*
	Example for ISO9660_getdir
	dirs = {"boot"}
*/
struct ISO9660_entity* ISO9660_get(char** dirs, uint8_t dirs_sz) {
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
	for(uint8_t dirs_i=0; dirs_i < dirs_sz; dirs_i++) {
		// Read the directory.
		ATAPI_read(
			(last_len % 2048 != 0) + (last_len / 2048),
			last_LBA
		);

		/*
			This would've been nicer with a struct, but as the length of
			each directory record is not fixed, I think it's not possible to do.
		*/

		// Run through the directory records of 'last' until one matches dirs[dirs_i].
		uint8_t found = 0;
		for(uint32_t i=0; i < last_len && !found; ) {
			// Check if the record length is ok. If it's zero, it couldn't be found.
			if(!
				*(uint8_t*)(
					ATAPI_PIO_BUFFER +
					i +
					ISO9660_DIR_RECORD_LENGTH
				)
			) break;

			// Get the filename.
			char* filename = (char*)(ATAPI_PIO_BUFFER + i + ISO9660_DIR_FILENAME);

			// Replace the semicolon with a null byte.
			for(uint32_t j=0; j < ISO9660_DIR_FILENAME_LENGTH; j++) {
				if(filename[j] == ';') {
					filename[j] = 0;
					break;
				}
			}

			// Do they match?
			if(strcmp(dirs[dirs_i], filename) == 0) {
				// Yep!
				found = 1;
				last_LBA = *(uint32_t*)(ATAPI_PIO_BUFFER + i + ISO9660_DIR_EAR_LBA);
				last_len = *(uint32_t*)(ATAPI_PIO_BUFFER + i + ISO9660_DIR_EAR_LENGTH);
			} else {
				// Nope, go for the next one
				i += *(uint8_t*)(
					ATAPI_PIO_BUFFER +
					i +
					ISO9660_DIR_RECORD_LENGTH
				);
			}
		}

		// Seems like there's no such directory.
		if(!found) {
			return (struct ISO9660_entity*)0;
		}
	}

	struct ISO9660_entity* ret = (struct ISO9660_entity*)(
		jmalloc(sizeof(struct ISO9660_entity))
	);
	ret->LBA = last_LBA;
	ret->length = last_len;

	return ret;
}

uint8_t* ISO9660_read(struct ISO9660_entity* entity) {
	ATAPI_read(
		(entity->length % 2048 != 0) + (entity->length / 2048),
		entity->LBA
	);
	return (uint8_t*)ATAPI_PIO_BUFFER;
}
