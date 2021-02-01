#include <kernel/drivers/term/term.hpp>
#include <kernel/klibc/stdio>
#include <kernel/klibc/stdlib.hpp>
#include <kernel/GDT/GDT.hpp>
#include <kernel/paging/paging.hpp>
#include <kernel/drivers/PIC/PIC.hpp>
#include <kernel/IDT/IDT.hpp>
#include <kernel/drivers/keyboard/keyboard.hpp>
#include <kernel/drivers/clock/clock.hpp>
#include <kernel/kernel_panic/kernel_panic.hpp>
#include <kernel/drivers/PCI/PCI.hpp>
#include <kernel/memutils/memutils.hpp>
#include <kernel/syscalls/syscalls.hpp>
#include <kernel/TSS/TSS.hpp>
#include <kernel/usermode/usermode.hpp>
#include <kernel/drivers/term/font/font.hpp>
#include <kernel/drivers/storage/FS/JOTAFS>
#include <kernel/drivers/storage/FS/JRAMFS>
#include <kernel/drivers/storage/FS/ISO9660>
#include <kernel/mounts/mounts.hpp>
#include <kernel/drivers/storage/FS/init_fs/init_fs.hpp>
#include <kernel/install/install.hpp>
#include <kernel/V86/V86.hpp>
#include <kernel/loader/loader.hpp>
#include <kernel/tasks/scheduler/scheduler.hpp>

void printSplash() {
	printf("\n");
	term_setFGC(0xB);

	printf("\n");
	FSRawChunk splash(readFile("/sys/splash.txt"));
	if(splash.good()) {
		printf("%S", splash.str());
		splash.destroy();
	} else {
		term_setFGC(0xC);
		printf("COULD NOT READ \"/sys/splash.txt\". SOMETHING HAS GONE REALLY WRONG. ABORTING.\n");
		while(true) {}
	}
	printf("\n");

	term_setFGC(0xA);
	printf("\n");
}

void showWarning() {
	FSRawChunk warning(readFile("/sys/shell.txt"));
	if(warning.good()) {
		term_setFGC(0xC);
		printf("%S", warning.str());
		term_setFGC(0xA);
		printf("\n");
	}
}

void printError(const string& err) {
	term_setFGC(0xC);
	printf("[ERROR] %S", err);
	term_setFGC(0xA);
	printf("\n");
}

extern "C" void kernel_main(void) {
	memutils_init();
	term_setFGC(0xA);
	term_init();

	printf("Setting GDT...\n");
	gdt_init();

	printf("Beginning paging...\n");
	paging_init();
	kernelPaging.use();
	go_paging();

	printf("Quieto parao!!!\nAhora serían las pruebas.");
	while(true);

	printf("Remapping PIC...\n");
	pic_init();

	printf("Setting IDT...\n");
	idt_init();

	printf("%d\n", sizeof(Scheduler::SchedulerTask)); while(true);

	printf("Scanning PCI devices...\n");
	list<PCI> devices(PCI_probe());

	printf("Loading drivers...\n");
	keyboard_init();
	clock_init();
	clock_enable();

	printf("Looking for IDE PCI devices...\n");
	// Just one ;)
	PCI ide_pci;
	bool IDE_found = false;
	for(auto& x : devices) {
		auto desc = x.getDescriptor();
		if(desc.classCode == 1 && desc.subclass == 1) {
			printf("Found IDE at %d:%d:%d\n", x.getBus(), x.getSlot(), x.getFunc());
			ide_pci = x;
			IDE_found = true;
			break;
		}
	}

	if(!IDE_found) {
		printf("Could not find an IDE PCI device.\n");
		while(true) {}
	}

	printf("Searching IDE drives...\n");
	PCI::Descriptor0 desc = ide_pci.getDescriptor();
	uint32_t* BARs = desc.BAR;
	// Global IDE instance (defined at IDE.cpp).
	ide.initialize(BARs[0], BARs[1], BARs[2], BARs[3], BARs[4]);

	// Check where we're booting from.
	uint8_t bootDriveID = *((uint8_t*)0x9000);
	bool isCD = (bootDriveID == 0xE0);

	printf("Mounting root...\n");
	ISO9660 iso;
	//JOTAFS jotafs;
	if(isCD) {
		// Search for the CD which contains the file IDENTIFY.TXT with the magic string.
		uint8_t driveid = 4;
		for(uint8_t i=0; i<4; ++i) {
			IDE::Device dev = ide.getDevices()[i];
			if(dev.Reserved != 1) {
				// Does not exist.
				continue;
			}
			if(dev.Type != 1) {
				// Not ATAPI.
				continue;
			}

			iso = i;
			FSRawChunk chunk = iso.readFile("IDENTIFY.TXT");
			if(!chunk.good()) {
				// IDENTIFY.TXT does not exist.
				continue;
			}

			if(string((const char*)chunk.get()) == "Yeah this is probably what you're looking for\n") {
				// Cool!
				driveid = i;
			}
		}

		if(driveid == 4)
			panic(KP_CD_NOT_FOUND);

		VFS* jramfs = new JRAMFS;
		mountRoot(jramfs);
		init_fs(jramfs, driveid);
	} else {
		// Read UUID with Virtual 8086.
		uint8_t buffer[512] = {0};
		V86_readSectorFromDrive(bootDriveID, JOTAFS_SECTOR_SUPERBLOCK, buffer);
		uint64_t UUID = (*(JOTAFS_model::SUPERBLOCK*)buffer).UUID;

		// Search for the ATA drive which matches the UUID.
		uint8_t driveid = 4;
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

			FSRawChunk chunk = ide.ATA_read(i, JOTAFS_SECTOR_SUPERBLOCK, 1);
			JOTAFS_model::SUPERBLOCK sb = *(JOTAFS_model::SUPERBLOCK*)(chunk.get());
			chunk.destroy();

			if(UUID == sb.UUID) {
				// Cool!
				driveid = i;
			}
		}

		if(driveid == 4)
			panic(KP_HDD_NOT_FOUND);

		// Assuming primary master (bad assumption).
		JOTAFS* jotafs = new JOTAFS(driveid);
		mountRoot(jotafs);
	}

	mount("/tmp/", new JRAMFS);


	printf("Loading font... ");
	FSRawChunk font = readFile("/sys/fonts/lat1-16/lat1-16.raw");
	if(font.good()) {
		setFont(font.get());
		loadFontToVGA();
		printf("\n");
	} else {
		printf("FAILED!\nFailling back to regular font.\n");
	}

	printSplash();
	syscalls_init();
	TSS_flush();

	// Run init.
	FSRawChunk init_chunk = readFile("/bin/core/init");
	if(!init_chunk.good()) {
		// TODO: Panic.
		printf(" {{ No init }} ");
		while(true) {}
	}

	Program init_p;
	init_p.parse(init_chunk.get());
	if(!init_p.loadDynamicLibraries()) {
		// TODO: Panic.
		printf("Not found: /lib/%S\n", init_p.getFailedDynamicLibrary());
		while(true) {}
	}

	init_p.load();
	init_p.relocate();

	Task init_task(init_p.getPages(), init_p.getEntryPoint());

	init_task.paging->use();
	if(!init_p.relocate2()) {
		// TODO: Panic.
		printf("Function not found: %S\n", init_p.getFailedRelocation());
		while(true) {}
	}
	kernelPaging.use();

	init_chunk.destroy();

	uint8_t cores = 1;

	// Let's go!
	initializeSchedulers(cores, init_task);

	printf("Cojones, todo fue bien.\n");
	while(true);

	schedulers[0].resume();

	// And that's all.
}
