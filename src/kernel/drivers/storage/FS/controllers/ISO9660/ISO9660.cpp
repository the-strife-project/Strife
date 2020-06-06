#include <kernel/drivers/storage/FS/controllers/ISO9660/ISO9660.hpp>
#include <kernel/klibc/stdlib.hpp>

FSRawChunk ISO9660::readFile(const string& path) {
	list<string> parts(path.split('/'));
	ISO9660_model::entity entity = model.get(parts);

	if(!entity.found)
		return FSRawChunk();

	return model.read(entity);
}

// Not cool. Easy tho.
void ISO9660::readFileTo(const string& path, FSRawChunk& theto) {
	FSRawChunk actualChunk = readFile(path);
	memcpy(theto.get(), actualChunk.get(), theto.getSize());
	actualChunk.destroy();
}

set<string> ISO9660::ls(const string& path) {
	list<string> parts = path.split('/');

	ISO9660_model::entity entity = model.get(parts);
	set<string> ret(model.getChildren_IamNotInsane(entity));
	ret.insert(".");
	ret.insert("..");

	return ret;
}
