# jotaOS

## Introduction
jotaOS is an x86 operating system made in C/C++ from scratch that I'm doing to learn. That's it. I'm not trying to make everything myself for any reason whatsoever but learning. There are so many topics in CS that I want to know in depth, and making an OS is a great way to touch all of them.

Please,

- Do not trust that I know what I'm doing.
- Do not copy/paste my code as it probably is improvable.
- Do not run jotaOS outside of a virtual machine. It will almost definitely make your computer explode.

The big parts implemented so far can be read in the next section. The things I haven't done yet which keep me interested can be seen [here](https://jlxip.net/jotaOS/TODO.html).

## What is done
So, here's what I've managed to do so far. None of these things are finished. Some are more polished than others: some I consider to be dealt with, and I will rarely have to touch them again; some are terribly incomplete, and have the bare minimum functionality to be here, but will get better with time as other parts get more relied upon them.

### Medium-high level
- Kernel C(++) library, using [liballoc](https://github.com/blanham/liballoc) as memory manager.

### Medium level
These are parts that do not have to deal directly with the hardware, or do so in a very limited way.

- Keyboard driver (Spanish layout).
- Kernel panics.
- Filesystems: ISO9660 and [JOTAFS](https://github.com/jlxip/jotaOS/blob/master/src/kernel/drivers/storage/FS/JOTAFS/README.md), my own filesystem.

### Low level
These can be programed in non-assembly programming languages (ignoring `in` and `out` instructions), but are still parts of an operating system that deal directly with the hardware.

- PCI driver.
- Management of VESA video modes.
- Programmed I/O drivers: ATA and ATAPI.
- Syscalls (well, some are more low level than others).

### Really low level
These can only be programmed in assembly, or require executing instructions that do not exist in C(++).

- My own bootloader, [JBoot](https://github.com/jlxip/jotaOS/blob/master/src/JBoot/README.md).
- GDT and IDT.
- Paging.
- Virtual 8086.
- User space.


## Compiling jotaOS
Before you can compile it, you need the following things:
- `i686-elf-gcc`
- `nasm`
- `qemu` and/or `bochs` and/or `VirtualBox`
- `python`, `genisoimage`
- `bison`, `flex`, `gmp3`, `mpc`, `mpfr`, `texinfo`, `xorriso`, `mtools`

In order to compile `i686-elf-gcc`, do the following (do NOT copy paste):
```
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"
mkdir -p /tmp/src && cd /tmp/src

wget https://ftp.gnu.org/gnu/binutils/binutils-2.32.tar.xz
wget https://ftp.gnu.org/gnu/gcc/gcc-9.1.0/gcc-9.1.0.tar.gz

tar xvf binutils-2.32.tar.xz
tar xvf gcc-9.1.0.tar.gz

cd binutils-2.32
./configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror

make -j8
make install

cd ../gcc-9.1.0
./configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc -j8
make all-target-libgcc -j8
make install-gcc
```

Now write `PATH="$HOME/opt/cross/bin:$PATH"` at the end of your shell profile (.bashrc, .zshrc...)

To compile jotaOS, open a new terminal in the directory, and run `./build`.

You can use `./run` to compile and run `qemu`, or `./debug` to compile and run `bochs`. There are equivalents for installing it in a virtual hard drive.

## Sources
I have used several resources in order to do this project.
- https://osdev.org <-- Mainly this one.
- https://www.youtube.com/channel/UCQdZltW7bh1ta-_nCH7LWYw
- http://ctyme.com/rbrown.htm
- Intel 64 and IA-32 Architectures Software Developer's Manual
