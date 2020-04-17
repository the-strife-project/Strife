#include <kernel/install/install.hpp>
#include <klibc/stdio>
#include <klibc/stdlib.hpp>
#include <kernel/drivers/term/term.hpp>
#include <kernel/drivers/storage/ATA_PIO/ATA_PIO.hpp>
#include <kernel/drivers/storage/ATAPI_PIO/ATAPI_PIO.hpp>
#include <kernel/drivers/storage/FS/ISO9660/ISO9660.hpp>
#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.hpp>
#include <klibc/string>
#include <klibc/STL/list>

static const char ERR_NOTFOUND[] = "[FAILED]\nISO might be badly created.\n";

// Read from ISO9660.
uint8_t* __readISO9660(const list<string>& path) {
	ISO9660_entity* entity = ISO9660_get(path);
	if(!entity) {
		printf(ERR_NOTFOUND);
		while(true) {}
	}
	uint8_t* contents = ISO9660_read(entity);
	jfree(entity);
	return contents;
}

// Read from ISO9660 and create a new file.
uint32_t __readAndNew(JOTAFS& jotafs, const list<string>& path, uint8_t filetype, uint16_t permissions) {
	ISO9660_entity* entity = ISO9660_get(path);
	if(!entity) {
		printf(ERR_NOTFOUND);
		while(true) {}
	}
	uint8_t* contents = ISO9660_read(entity);

	uint32_t ret = jotafs.newfile(entity->length, contents, JOTAFS_SUPERUSER_UID, filetype, permissions);

	jfree(contents);
	jfree(entity);
	return ret;
}

// Copy from ISO9660 into a new file in a directory.
inline void __copy(JOTAFS& jotafs, const list<string>& path, uint8_t filetype, uint16_t permissions,
				   const string& name, JOTAFS::DIR& dir) {
	uint32_t inode_n = __readAndNew(jotafs, path, filetype, permissions);
	dir.addChild(name, inode_n);
}

void install() {
	printf("%s", "You're booting from a CD!\n"
		"This means that you got jotaOS booting, which is quite cool.\n"
		"However, you won't be able to do anything but install the OS onto the hard disk.\n\n"
		"Note that you should ONLY be running jotaOS in a virtual machine,\n"
		"as it's incompatible with any other operating system.\n\n");

	// Check the drive is there.
	ATA primarymaster(1, 0x1F0);
	JOTAFS jotafs(primarymaster);
	if(!jotafs.getStatus()) {
		printf("You must insert an HDD into the primary master ATA drive.\n"
		"Then, please reboot.\n");
		while(1) {}
	}

	// Check if the disk is already formatted with JOTAFS.
	if(jotafs.checkSignature()) {
		printf("jotaOS seems to be already installed on the hard disk.\n"
			"Proceed with caution.\n\n"
		);
	}

	printf("%s", "Now, write \"yes\" without quotes to install jotaOS into the\n"
		"primary master ATA drive. Any other input will stop the installation.\n\n");

	printf("-> ");
	showCursor();
	string installInput = readLine();
	hideCursor();
	if(installInput != "yes") {
		// Stop!
		printf("\nInstallation stopped.\n");
		printf("You can reboot the machine now.\n");
		while(1) {}
	}

	printf("\nHere we go!\n");


	// Format disk.
	printf("Formatting disk... ");
	jotafs.format();
	printf("[OK]\n");


	// Copy MBR.
	printf("Copying boot sector... ");
	list<string> stage1_p(string("BOOT/HDDS1.BIN").split('/'));
	uint8_t* stage1 = __readISO9660(stage1_p);
	jotafs.writeBoot(stage1);
	jfree(stage1);
	printf("[OK]\n");


	// Copy JBoot's second stage.
	printf("Copying JBoot's second stage... ");
	list<string> stage2_p(string("BOOT/HDDS2.BIN").split('/'));
	__readAndNew(jotafs, stage2_p, JOTAFS::FILETYPE::SYSTEM, 0);

	printf("[OK]\n");


	// Copy the kernel.
	printf("Copying the kernel... ");
	list<string> kernel_p(string("BOOT/KERNEL.BIN").split('/'));
	ISO9660_entity* kernel_e = ISO9660_get(kernel_p);
	if(!kernel_e) {
		printf("[FAILED]\n"
		"This error is impossible.\n"
		);
		while(1) {}
	}
	/*
		The kernel is actually too big to be read by ISO9660_read.
		We have to read it granularly.

		Btw, this sometimes fires a 0x20 interrupt. Probably has to do
		with weird V86 stuff. Don't know. Don't care. I'll get rid of
		this disgusting ATAPI driver soon.
	*/
	uint8_t* kernel = (uint8_t*)jmalloc(kernel_e->length + 2048);
	ATAPI_granularread(1+(kernel_e->length / 2048), kernel_e->LBA, kernel);
	jotafs.newfile(kernel_e->length, kernel, JOTAFS_SUPERUSER_UID, JOTAFS::FILETYPE::SYSTEM, 0);
	jfree(kernel);
	jfree(kernel_e);
	printf("[OK]\n");


	printf("Creating directory tree... ");

	// Root (inode will be JOTAFS_INODE_ROOT=3).
	JOTAFS::DIR root = jotafs.newdir(JOTAFS_SUPERUSER_UID, JP_USER | JP_GR | JP_GX | JP_OR | JP_OX, JOTAFS::RESERVED_INODE::ROOT);
	// "/bin/"
	JOTAFS::DIR dir_bin = jotafs.newdir(JOTAFS_SUPERUSER_UID, JP_USER | JP_GR | JP_GX | JP_OR | JP_OX, JOTAFS::RESERVED_INODE::ROOT);
	root.addChild("bin", dir_bin.getInodeNumber());
	// "/bin/core"
	JOTAFS::DIR dir_bin_core = jotafs.newdir(JOTAFS_SUPERUSER_UID, JP_USER | JP_GR | JP_GX | JP_OR | JP_OX, dir_bin.getInodeNumber());
	dir_bin.addChild("core", dir_bin_core.getInodeNumber());
	// "/sys/"
	JOTAFS::DIR dir_sys = jotafs.newdir(JOTAFS_SUPERUSER_UID, JP_UR | JP_UX, JOTAFS::RESERVED_INODE::ROOT);
	root.addChild("sys", dir_sys.getInodeNumber());
	// "/sys/boot"
	dir_sys.addChild("boot", JOTAFS::RESERVED_INODE::JBOOT2);
	// "/sys/kernel"
	dir_sys.addChild("kernel", JOTAFS::RESERVED_INODE::KERNEL);
	// "/sys/fonts/"
	JOTAFS::DIR dir_sys_fonts = jotafs.newdir(JOTAFS_SUPERUSER_UID, JP_UR | JP_UX, dir_sys.getInodeNumber());
	dir_sys.addChild("fonts", dir_sys_fonts.getInodeNumber());
	// "/sys/fonts/lat1-16/"
	JOTAFS::DIR dir_sys_fonts_lat116 = jotafs.newdir(JOTAFS_SUPERUSER_UID, JP_UR | JP_UX, dir_sys_fonts.getInodeNumber());
	dir_sys_fonts.addChild("lat1-16", dir_sys_fonts_lat116.getInodeNumber());

	printf("[OK]\n");


	// Copy the MSS.
	printf("Copying MSS... ");
	list<string> mss_p(string("MSS.BIN").split('/'));
	__copy(jotafs, mss_p, JOTAFS::FILETYPE::REGULAR_FILE, JP_USER | JP_GR | JP_GX | JP_OR | JP_OX, "mss", dir_bin_core);
	printf("[OK]\n");


	// Other stuff
	printf("Copying some other stuff... ");
	list<string> font_p(string("FONT/LAT1/LAT1.RAW").split('/'));
	__copy(jotafs, font_p, JOTAFS::FILETYPE::REGULAR_FILE, JP_UR, "lat1-16.font", dir_sys_fonts_lat116);

	list<string> font_readme_p(string("FONT/LAT1/README.TXT").split('/'));
	__copy(jotafs, font_readme_p, JOTAFS::FILETYPE::REGULAR_FILE, JP_UR, "README.txt", dir_sys_fonts_lat116);
	printf("[OK]\n");

	printf("\nInstallation successful!\nYou can reboot now.");
	while(1) {}
}
