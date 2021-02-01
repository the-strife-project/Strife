#include <kernel/install/install.hpp>
#include <kernel/klibc/stdio>
#include <kernel/klibc/stdlib.hpp>
#include <kernel/drivers/term/term.hpp>
#include <kernel/drivers/storage/FS/models/JOTAFS/JOTAFS.hpp>
#include <string>
#include <list>
#include <kernel/drivers/storage/FS/VFS.hpp>
#include <kernel/drivers/storage/FS/JOTAFS>
#include <kernel/drivers/storage/FS/init_fs/init_fs.hpp>

static const char ERR_NOTFOUND[] = "[FAILED]\nISO might be badly created.\n";

ISO9660 iso;

// Read from ISO9660.
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

void install(ISO9660& _iso) {
	iso = _iso;
	printf("Please know that you should ONLY be installing jotaOS in a virtual machine,\n"
		"as it's incompatible with any other operating system.\n\n");

	// Get ATA drives.
	vector<uint8_t> drives;
	for(uint8_t i=0; i<4; ++i) {
		IDE::Device dev = ide.getDevices()[i];
		if(dev.Reserved != 1) {
			// Does not exist.
			continue;
		}
		if(dev.Type != 0) {
			// Not ATA.
			continue;
		}

		drives.push_back(i);
	}

	uint8_t driveid = 4;
	if(drives.size() == 0) {
		printf("You must insert an HDD. Then, please reboot.\n");
		return;
	} else if(drives.size() == 1) {
		driveid = drives[0];
		printf("Only one hard drive connected. Using that.\n\n");
	} else {
		printf("Select the drive you want to install jotaOS onto:\n");
		for(uint8_t i=0; i<drives.size(); ++i) {
			printf(" [%d] (", i+1);

			IDE::Device dev = ide.getDevices()[drives[i]];
			uint32_t size = dev.Size / 2;
			// Currently, KiB.
			if(size >= 1024*1024) {
				size >>= 20;
				printf("%d GiB, ", size);
			} else if(size >= 1024) {
				size >>= 10;
				printf("%d MiB, ", size);
			}
			if(dev.Channel == 0)
				printf("primary ");
			else
				printf("secondary ");
			if(dev.Drive == 0)
				printf("master) ");
			else
				printf("slave) ");
			printf("%S\n", string((const char*)dev.Model));
		}
		printf(" [x] Stop installation\n\n[x] ");
		string hddInput = readLine();

		if(!hddInput.isNatural()) {
			printf("Stopped.\n");
			return;
		}

		driveid = atoi(hddInput) - 1;
		if(driveid >= drives.size()) {
			printf("Stopped.\n");
			return;
		}

		driveid = drives[driveid];
	}

	 JOTAFS_model jotafs(driveid);

	// Check if the disk is already formatted with JOTAFS.
	if(jotafs.checkSignature()) {
		printf("jotaOS seems to be already installed on the hard disk.\n"
			"Proceed with caution.\n\n"
		);
	}

	printf("Now, write \"yes\" to install jotaOS. Any other input will stop the installation.\n\n");

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
	__readAndNew(jotafs, "BOOT/KERNEL.BIN", JOTAFS_model::FILETYPE::SYSTEM, 0);
	printf("[OK]\n");

	printf("Initializing filesystem... ");

	// Root, inode will be JOTAFS_INODE_ROOT (3).
	jotafs.newdir(JOTAFS_SUPERUSER_UID, JP_USER | JP_GR | JP_GX | JP_OR | JP_OX, JOTAFS_model::RESERVED_INODE::ROOT);

	JOTAFS* wrapper = new JOTAFS(driveid);
	init_fs(wrapper, INIT_FS_UNUSED_PARAMETER);

	// This cannot be done in an abstract way so I have to do it with the model.
	JOTAFS_model::DIR dir_sys(&jotafs, jotafs.find("/sys"));
	dir_sys.addChild("boot", JOTAFS_model::RESERVED_INODE::JBOOT2);
	dir_sys.addChild("kernel", JOTAFS_model::RESERVED_INODE::KERNEL);

	printf("[OK]\n");

	printf("\nInstallation successful!\nYou can reboot now.\n");
}
