#include <kernel/drivers/storage/FS/controllers/ISO9660/ISO9660.hpp>
#include <kernel/klibc/stdlib.hpp>

FSRawChunk ISO9660::readFile(const string& path, bool big) {
	list<string> parts(path.split('/'));
	ISO9660_entity* entity = ISO9660_get(parts);

	if(!entity)
		return FSRawChunk();

	uint64_t size = entity->length;

	uint8_t* data;
	if(big)
		data = ISO9660_granularread(entity);
	else
		data = ISO9660_read(entity);

	delete entity;
	return FSRawChunk(data, size);
}

void ISO9660::readFileTo(const string& path, FSRawChunk& theto) {
	// TODO: With DMA this would be fast. Right now it's pure shit.
	FSRawChunk actualChunk = readFile(path, false);
	memcpy(theto.get(), actualChunk.get(), theto.getSize());
	actualChunk.destroy();
}

set<string> ISO9660::ls(const string& path) {
	list<string> parts = path.split('/');

	ISO9660_entity* entity = ISO9660_get(parts);
	set<string> ret(ISO9660_getChildren_IamNotInsane(entity));
	ret.insert(".");
	ret.insert("..");

	delete entity;

	return ret;
}
