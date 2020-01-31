#include <kernel/install/install.h>
#include <klibc/stdio.h>
#include <klibc/stdlib.h>
#include <kernel/drivers/term/term.h>
#include <kernel/drivers/storage/ATA_PIO/ATA_PIO.h>
#include <kernel/drivers/storage/ATAPI_PIO/ATAPI_PIO.h>
#include <kernel/drivers/storage/FS/ISO9660/ISO9660.h>
#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.h>
#include <klibc/string>
#include <klibc/STL/list>

void install() {
	printf("%s", "You're booting from a CD!\n"
		"This means that you got jotadOS booting, which is quite cool.\n"
		"However, you won't be able to do anything but install the OS onto the hard disk.\n\n"
		"Note that you should ONLY be running jotadOS in a virtual machine,\n"
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
		printf("jotadOS seems to be already installed on the hard disk.\n"
			"Proceed with caution.\n\n"
		);
	}

	printf("%s", "Now, write \"yes\" without quotes to install jotadOS into the\n"
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
	list<string> stage1_p; stage1_p.push_back("BOOT"); stage1_p.push_back("HDDS1.BIN");
	struct ISO9660_entity* stage1_e = ISO9660_get(stage1_p);
	if(!stage1_e) {
		printf("[FAILED]\n"
		"Couldn't find HDDS1.BIN in the BOOT directory of the CD.\n"
		"Installation stopped.\n"
		);
		while(1) {}
	}
	uint8_t* stage1 = ISO9660_read(stage1_e);
	jfree(stage1_e);
	jotafs.writeBoot(stage1);
	printf("[OK]\n");

	// Copy JBoot's second stage.
	printf("Copying JBoot's second stage... ");
	list<string> stage2_p; stage2_p.push_back("BOOT"); stage2_p.push_back("HDDS2.BIN");
	struct ISO9660_entity* stage2_e = ISO9660_get(stage2_p);
	if(!stage2_e) {
		printf("[FAILED]\n"
		"Couldn't find HDDS2.BIN in the BOOT directory of the CD.\n"
		"Installation stopped."
		);
		while(1) {}
	}
	uint8_t* stage2 = ISO9660_read(stage2_e);
	jotafs.newfile(stage2_e->length, stage2, 0, JOTAFS_FILETYPE_REGULAR_FILE, 0);
	jfree(stage2_e);
	printf("[OK]\n");

	// Copy the kernel.
	printf("Copying the kernel... ");
	list<string> kernel_p; kernel_p.push_back("BOOT"); kernel_p.push_back("KERNEL.BIN");
	struct ISO9660_entity* kernel_e = ISO9660_get(kernel_p);
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
	jotafs.newfile(kernel_e->length, kernel, 0, JOTAFS_FILETYPE_REGULAR_FILE, 0);
	jfree(kernel);
	jfree(kernel_e);
	printf("[OK]\n");


	printf("Creating directory tree... ");

	// Root (inode will be JOTAFS_INODE_ROOT=3).
	// TODO: Probably there's an issue with newdir or inode allocation.
	/*JOTAFS::DIR root = */jotafs.newdir(0, JP_USER | JP_GR | JP_GX | JP_OR | JP_OX);
	// "/bin/"
	/*JOTAFS::DIR dir_bin = */jotafs.newdir(0, JP_USER | JP_GR | JP_GX | JP_OR | JP_OX);
	//root.addChild("bin", dir_bin.getInodeNumber());
	// "/bin/core"
	/*JOTAFS::DIR dir_bin_core = *///jotafs.newdir(0, JP_USER | JP_GR | JP_GX | JP_OR | JP_OX);
	//dir_bin.addChild("core", dir_bin_core.getInodeNumber());

	printf("[OK]\n");


	// Copy the MSS.
	printf("Copying MSS... ");
	list<string> mss_p; mss_p.push_back("MSS.BIN");
	struct ISO9660_entity* mss_e = ISO9660_get(mss_p);
	if(!mss_e) {
		printf("[FAILED]\n"
		"MSS.bin could not be found."
		);
		while(1) {}
	}
	uint8_t* mss = ISO9660_read(mss_e);
	/*uint32_t mss_inoden = */jotafs.newfile(mss_e->length, mss, 0, JOTAFS_FILETYPE_REGULAR_FILE, 0);
	//dir_bin_core.addChild("mss", mss_inoden);
	jfree(mss_e);

	printf("[OK]\n");

	printf("\nInstallation successful!\nYou can reboot now.");
	while(1) {}
}
