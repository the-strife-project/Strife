#include <kernel/install/install.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <kernel/drivers/term/term.h>
#include <kernel/drivers/storage/ATA_PIO/ATA_PIO.h>
#include <kernel/drivers/storage/FS/ISO9660/ISO9660.h>
#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.h>
#include <kernel/drivers/storage/ATAPI_PIO/ATAPI_PIO.h>

void install() {
	printf("%s", "You're booting from a CD!\n"
		"This means that you got jotadOS booting, which is quite cool.\n"
		"However, you won't be able to do anything but install the OS onto the hard disk.\n\n"
		"Note that you should ONLY be running jotadOS in a virtual machine,\n"
		"as it's incompatible with any other operating system.\n\n");

	// Check the drive is there.
	struct ATA_INTERFACE* primarymaster = newATA(1, 0x1F0);
	if(JOTAFS_init(primarymaster) != 0) {
		printf("You must insert an HDD into the primary master ATA drive.\n"
		"Then, please reboot.\n");
		while(1) {}
	}

	printf("%s", "Now, write \"yes\" without quotes to install jotadOS into the\n"
		"primary master ATA drive. Any other input will stop the installation.\n\n");

	printf("-> ");
	showCursor();
	char* installInput = readLine();
	if(strcmp("yes", installInput) != 0) {
		// Stop!
		printf("\nInstallation stopped.\n");
		printf("You can reboot the machine now.\n");
		while(1) {}
	}

	printf("\nHere we go!\n");

	// Read MBR.
	printf("Copying MBR... ");
	//char* stage1_p[] = {"BOOT", "HDDS1.BIN"};
	//struct ISO9660_entity* stage1_e = ISO9660_get(stage1_p, 2);
	ATAPI_read(1, 0x10);
	/*if(!stage1_e) {
		printf("[FAILED]\n"
		"Couldn't find HDDS1.BIN in the BOOT directory of the CD.\n"
		"Installation stopped.\n"
		);
		while(1) {}
	}*/
	//uint8_t* stage1 = ISO9660_read(stage1_e);

	//uint8_t* aux = jmalloc(512);
	//for(int i=0; i<512; i++) aux[i] = stage1[i];

	// Copy MBR.
	JOTAFS_writeMBR((uint8_t*)primarymaster);

	printf("[OK]\n");

	//printf("\nInstallation successful!\n");
	while(1) {}
}
