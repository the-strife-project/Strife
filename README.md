# jotaOS

## Introduction
jotaOS is an x86 operating system made in C/C++ from scratch that I'm doing to learn. That's it. I'm not trying to make everything myself for any reason whatsoever but learning. There are so many topics in CS that I want to know in depth, and making an OS is a great way to touch all of them.

Please,

- Do not trust that I know what I'm doing.
- Do not copy/paste my code as it probably is improvable.
- Do not run jotaOS outside of a virtual machine. It will almost definitely make your computer explode.
- Read [the wiki](https://github.com/jotaOS/jotaOS/wiki).

The big parts implemented so far can be read in the next section. The things I haven't done yet which keep me interested can be seen [here](https://jlxip.net/jotaOS/TODO.html).

## What is done
So, here's what I've managed to do so far. None of these things are finished. Some are more polished than others: some I consider to be dealt with, and I will rarely have to touch them again; some are terribly incomplete, and have the bare minimum functionality to be here, but will get better with time as other parts get more relied upon them.

### Medium-high level
- Kernel C++ library, using [liballoc](https://github.com/blanham/liballoc) as memory manager.

### Medium level
These are parts that do not have to deal directly with the hardware, or do so in a very limited way.

- Keyboard driver (Spanish layout).
- Kernel panics.
- Filesystems: ISO9660, [JOTAFS](https://github.com/jotaOS/jotaOS/wiki/JOTAFS), and [JRAMFS](https://github.com/jotaOS/jotaOS/wiki/JRAMFS).

### Low level
These can be programed in non-assembly programming languages (ignoring `in` and `out` instructions), but are still parts of an operating system that deal directly with the hardware.

- PCI driver.
- Management of VESA video modes.
- Programmed I/O drivers: ATA and ATAPI.
- Syscalls (well, some are more low level than others).

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
