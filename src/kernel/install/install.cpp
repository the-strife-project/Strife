#include <kernel/install/install.hpp>
#include <kernel/klibc/stdio>
#include <kernel/klibc/stdlib.hpp>
#include <kernel/drivers/term/term.hpp>
#include <kernel/drivers/storage/FS/models/JOTAFS/JOTAFS.hpp>
#include <kernel/klibc/STL/string>
#include <kernel/klibc/STL/list>
#include <kernel/drivers/storage/FS/VFS.hpp>
#include <kernel/drivers/storage/FS/JOTAFS>
#include <kernel/drivers/storage/FS/ISO9660>
#include <kernel/drivers/storage/FS/init_fs/init_fs.hpp>

static const char ERR_NOTFOUND[] = "[FAILED]\nISO might be badly created.\n";

// Read from ISO9660.
ISO9660 iso;
FSRawChunk __readISO9660(const string& path) {
	FSRawChunk contents = iso.readFile(path);

	if(!contents.good()) {
		printf(ERR_NOTFOUND);
		while(true) {}
	}

	return contents;
}


// Read from ISO9660 and create a new file.
uint32_t __readAndNew(JOTAFS_model& jotafs, const string& path, uint8_t filetype, uint16_t permissions) {
	FSRawChunk contents = __readISO9660(path);

	uint32_t ret = jotafs.newfile(contents.getSize(), contents.get(), JOTAFS_SUPERUSER_UID, filetype, permissions);
	contents.destroy();
	return ret;
}

void install() {
	printf("Please know that you should ONLY be installing jotaOS in a virtual machine,\n"
		"as it's incompatible with any other operating system.\n\n");

	// Check the drive is there.
	ATA primarymaster(1, 0x1F0);	// <-- TODO
	JOTAFS_model jotafs(primarymaster);
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

	printf("Now, write \"yes\" to install jotaOS into the primary master ATA drive.\n"
		"Any other input will stop the installation.\n\n");

	printf("-> ");
	string installInput = readLine();
	if(installInput != "yes") {
		printf("\nInstallation stopped.\n");
		return;
	}

	printf("\nHere we go!\n");


	// Format disk.
	printf("Formatting disk... ");
	jotafs.format();
	printf("[OK]\n");


	// Copy MBR.
	printf("Copying boot sector... ");
	FSRawChunk stage1 = __readISO9660("BOOT/HDDS1.BIN");
	jotafs.writeBoot(stage1.get());
	stage1.destroy();
	printf("[OK]\n");


	// Copy JBoot's second stage.
	printf("Copying JBoot's second stage... ");
	__readAndNew(jotafs, "BOOT/HDDS2.BIN", JOTAFS_model::FILETYPE::SYSTEM, 0);
	printf("[OK]\n");


	// Copy the kernel.
	printf("Copying the kernel... ");
	FSRawChunk kernel = iso.readFile("BOOT/KERNEL.BIN", true);

	if(!kernel.good()) {
		printf("[FAILED]\nThis error is impossible.\n");
		while(true) {}
	}

	jotafs.newfile(kernel.getSize(), kernel.get(), JOTAFS_SUPERUSER_UID, JOTAFS_model::FILETYPE::SYSTEM, 0);

	kernel.destroy();
	printf("[OK]\n");

	printf("Initializing filesystem... ");

	// Root, inode will be JOTAFS_INODE_ROOT (3).
	jotafs.newdir(JOTAFS_SUPERUSER_UID, JP_USER | JP_GR | JP_GX | JP_OR | JP_OX, JOTAFS_model::RESERVED_INODE::ROOT);

	JOTAFS* wrapper = new JOTAFS(primarymaster);
	init_fs(wrapper);

	// This cannot be done in an abstract way so I have to do it with the model.
	JOTAFS_model::DIR dir_sys(&jotafs, jotafs.find("/sys"));
	dir_sys.addChild("boot", JOTAFS_model::RESERVED_INODE::JBOOT2);
	dir_sys.addChild("kernel", JOTAFS_model::RESERVED_INODE::KERNEL);

	printf("[OK]\n");

	printf("\nInstallation successful!\nYou can reboot now.\n");
}
