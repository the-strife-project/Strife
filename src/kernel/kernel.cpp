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
#include <kernel/asm.hpp>
#include <kernel/PCI/PCI.hpp>
#include <kernel/memutils/memutils.hpp>
#include <kernel/syscalls/syscalls.hpp>
#include <kernel/TSS/TSS.hpp>
#include <kernel/usermode/usermode.hpp>
#include <kernel/drivers/term/font/font.hpp>
#include <kernel/drivers/storage/FS/JOTAFS>
#include <kernel/drivers/storage/FS/JRAMFS>
#include <kernel/mounts/mounts.hpp>
#include <kernel/drivers/storage/FS/init_fs/init_fs.hpp>
#include <kernel/install/install.hpp>

void printSplash() {
	printf("\n");
	term_setFGC(0xB);

	printf("\n");
	FSRawChunk splash(readFile("/sys/splash.txt", false));
	if(splash.good()) {
		printf("%S", splash.str());
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
	FSRawChunk warning(readFile("/sys/shell.txt", false));
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
	paging_enable();

	printf("Remapping PIC...\n");
	pic_init();

	printf("Setting IDT...\n");
	idt_init();

	printf("Loading drivers...\n");
	keyboard_init();

	//clock_init();
	//clock_start();

	// Check where we're booting from.
	uint8_t bootDriveID = *((uint8_t*)0x9000);
	bool isCD = (bootDriveID == 0xE0); // This check will be made good soon. Right now it's horrible.

	printf("Mounting root...\n");
	if(isCD) {
		VFS* jramfs = new JRAMFS;
		mountRoot(jramfs);
		init_fs(jramfs);
	} else {
		// Assuming primary master (bad assumption).
		ATA primaryMaster(1, 0x1F0);	// <- Nice magic number there bud.
		JOTAFS* jotafs = new JOTAFS(primaryMaster);
		mountRoot(jotafs);
	}

	printf("Loading font... ");
	FSRawChunk font = readFile("/sys/fonts/lat1-16/lat1-16.raw", false);
	if(font.good()) {
		setFont(font.get());
		loadFontToVGA();
		printf("\n");
	} else {
		printf("FAILED!\nFailling back to regular font.\n");
	}

	printSplash();

	/*
		--- All the code below this point will be certainly changed ---
	*/

	showWarning();

	string workingDirectory("/");
	while(true) {
		// Read.
		term_setFGC(0xE);
		printf("[%S] $ ", workingDirectory);
		term_setFGC(0xA);
		string line(readLine());

		// Parse.
		list<string> parts = line.split(' ');
		string command = *parts.begin();
		auto it = parts.begin();
		++it;
		string arg;
		if(it != parts.end())
			arg = *it;

		// Interpret.
		if(command == "") {
			// Do nothing.
		} else if(command == "help") {
			if(arg == "") {
				printf("Usage: help <command>\n");
			} else if(arg == "help") {
				printf("I see you like recursion.\n");
			} else if(arg == "echo") {
				printf("Repeats whatever you tell it to.\n");
				printf("Usage: echo <something>\n");
			} else if(arg == "ls") {
				printf("Shows the contents of the current directory.\n");
				printf("Usage: ls\n");
			} else if(arg == "cd") {
				printf("Change current directory.\n");
				printf("Usage: cd <path>\n");
			} else if(arg == "mkd") {
				printf("Creates a directory under the current one.\n");
				printf("Usage: mkd <name>\n");
			} else if(arg == "rd") {
				printf("Prints the contents of a file.\n");
				printf("Usage: rd <path>\n");
			} else if(arg == "install") {
				printf("Installs jotaOS onto the hard disk.\n");
				printf("Usage: install\n");
			} else if(arg == "reboot") {
				printf("You tell me.\n");
				printf("Usage: reboot\n");
			} else {
				printError("Bruh what is that?");
			}
		} else if(command == "echo") {
			parts.pop_front();
			for(auto const& x : parts)
				printf("%S ", x);
			printf("\n");
		} else if(command == "ls") {
			for(auto const& x : ls(workingDirectory))
				printf("%S\n", x);
		} else if(command == "cd") {
			if(arg == "") {
				printf("Usage: cd <path>\n");
			} else {
				// Does it exist?
				string path;
				if(arg[0] != '/') {
					path = workingDirectory;
					path += '/';
				}
				path += arg;

				path = shortenPath(path);
				if(isDir(path)) {
					workingDirectory = path;
				} else {
					printError("Path not found!");
				}
			}
		} else if(command == "mkd") {
			if(arg == "") {
				printf("Usage: mkd <name>\n");
			} else {
				bool bad = false;
				for(auto const& x : arg) {
					if(x == '/') {
						bad = true;
						break;
					}
				}

				if(bad) {
					printError("No slashes!!!");
				} else {
					string path = workingDirectory;
					path += '/';
					path += arg;
					mkd(path);
				}
			}
		} else if(command == "rd") {
			if(arg == "") {
				printf("Usage: rd <path>\n");
			} else {
				// Does the file exist tho?
				string path;
				if(arg[0] != '/') {
					path = workingDirectory;
					path += '/';
				}
				path += arg;

				if(!isFile(path)) {
					printError("That file doesn't exist, bro.");
				} else {
					FSRawChunk contents(readFile(path, true));
					if(!contents.good()) {
						printError("Woah, what happened there? You found a bug! Write down immediately what you did and send it to me.");
					} else {
						printf("%S\n", contents.str());
					}
				}
			}
		} else if(command == "install") {
			printf("Running installation program...\n\n");
			install();
		} else if(command == "reboot") {
			// Temporal way to reboot.
			uint8_t good = 0x02;
			while (good & 0x02)
				good = inb(0x64);
			outb(0x64, 0xFE);

			printf("Could not reboot. Please pull the cord.\n");
			while(true) {}
		} else {
			printError("What is that?");
		}
	}

	/*// Enable syscalls.
	syscalls_init();

	// Load the TSS.
	TSS_flush();

	// Load and run MSS.
	uint32_t mss = paging_allocPages(1);
	paging_setUser(mss, 1);
	FSRawChunk mss_chunk((uint8_t*)mss, PAGE_SIZE);
	if(isCD)
		readFileTo("MSS.BIN", mss_chunk);
	else
		readFileTo("/bin/core/mss", mss_chunk);

	if(!mss_chunk.good()) {
		printf("MSS not found!");
		while(true) {}
	}

	jump_usermode(mss);

	printf("\n[[[ MSS RETURNED?!?!?! ]]]");
	while(1) {}*/
}
