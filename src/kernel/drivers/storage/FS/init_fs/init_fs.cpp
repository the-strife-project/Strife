#include <kernel/drivers/storage/FS/init_fs/init_fs.hpp>
#include <kernel/drivers/storage/FS/ISO9660>
#include <kernel/mounts/mounts.hpp>

/*
	If JRAMFS, link. Otherwise, copy.
*/
inline void copyOrLink(bool ram, const string& orig, const string& dst) {
	if(ram)
		link(dst, orig);	// Lmao I should probably turn this around.
	else
		copy(orig, dst);
}

void init_fs(VFS* vfs, uint8_t driveid) {
	bool jramfs = vfs->isRAM();

	// Where do we start from.
	const string orig = "/mnt/CDROM";
	// Where's the FS we want to initialize.
	string dst;
	if(jramfs) {
		// Mount CD.
		mkd("/mnt/");
		dst = "/";
		mkd(orig);
		mount(orig, new ISO9660(driveid));
	} else {
		// At this point in time, /mnt is already there.
		dst = "/mnt/HDD";
		mkd(dst);
		mount(dst, vfs);
	}

	mkd(dst + "/sys/");
	mkd(dst + "/sys/fonts/");
	mkd(dst + "/sys/fonts/lat1-16/");

	copyOrLink(jramfs, orig+"/FONT/LAT1/README.TXT", dst+"/sys/fonts/lat1-16/readme.txt");
	copyOrLink(jramfs, orig+"/FONT/LAT1/LAT1.RAW", dst+"/sys/fonts/lat1-16/lat1-16.raw");
	copyOrLink(jramfs, orig+"/SPLASH.TXT", dst+"/sys/splash.txt");
	copyOrLink(jramfs, orig+"/SHELL.TXT", dst+"/sys/shell.txt");

	//mkd("/bin/");
	//mkd("/bin/core/");
	// MSS here.
}
