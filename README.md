# THIS BRANCH IS KEPT FOR ARCHIVAL PURPOSES. DO NOT TRY TO COMPILE IT

Today is February the 1st 2021, and I have decided to rewrite the whole thing from scratch. Why? There are some *MAJOR* changes that are about to happen to jotaOS, these are:
- Changing the whole type of kernel. From the current monolithic to a microkernel.
- Changing the whole architecture. From x86, to x86_64.
- Due to the previous change, changing the bootloader. From JBoot, the one I made, to Limine.

These changes are not worth making, specially not at the current state of jotaOS, and it's extremely worth indeed starting from scratch. This branch is only kept for archival purposes. Don't try to do anything with it besides maybe look at the commit history so you can really appreciate how much time I've dedicated to this project. This very commit will be the last, and contains PARTIAL and INCOMPLETE changes to the switch to a microkernel, this includes the scheduler and a whole bunch of stuff. That's it, see you in the new branch!


# jotaOS

## Introduction
jotaOS is an x86 operating system made in C/C++ from scratch that I'm doing to learn. That's it. I'm not trying to make everything myself for any reason whatsoever but learning. There are so many topics in CS that I want to know in depth, and making an OS is a great way to touch all of them.

Please,

- Do not trust that I know what I'm doing.
- Do not copy/paste my code as it probably is improvable.
- Do not run jotaOS outside of a virtual machine. It will almost definitely make your computer explode.
- Read [the wiki](https://github.com/jotaOS/jotaOS/wiki) for downloading/building instructions and more!

The big parts implemented so far can be read in the next section. The things I haven't done yet which keep me interested can be seen [here](https://jlxip.net/jotaOS/TODO.html).

## What is done
So, here's what I've managed to do so far. Some are more polished than others: some I consider to be dealt with, and I will rarely have to touch them again; some are terribly incomplete, and have the bare minimum functionality to be here, but will get better with time as other parts get more relied upon them.

### Medium-high level
- Kernel C++ library, using [liballoc](https://github.com/blanham/liballoc) as memory manager.

### Medium level
These are parts that do not have to deal directly with the hardware, or do so in a very limited way.

- Keyboard driver (Spanish layout).
- Kernel panics.
- Filesystems: ISO9660, [JOTAFS](https://github.com/jotaOS/jotaOS/wiki/JOTAFS), and [JRAMFS](https://github.com/jotaOS/jotaOS/wiki/JRAMFS).
- Binary and shared library loader.

### Low level
These can be programed in non-assembly programming languages (ignoring `in` and `out` instructions), but are still parts of an operating system that deal directly with the hardware.

- PCI driver.
- Management of VESA video modes.
- IDE driver (PIO).
- Syscalls (some are more low level than others).

### Really low level
These can only be programmed in assembly, or require executing instructions that do not exist in C(++).

- My own bootloader, [JBoot](https://github.com/jotaOS/JBoot).
- GDT and IDT.
- Paging.
- Virtual 8086.
- User space.

## Sources
I have used several resources in order to do this project.
- https://osdev.org <-- Mainly this one.
- https://www.youtube.com/channel/UCQdZltW7bh1ta-_nCH7LWYw
- http://ctyme.com/rbrown.htm
- Intel 64 and IA-32 Architectures Software Developer's Manual
