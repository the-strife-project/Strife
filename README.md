# jotadOS

## Introduction
jotadOS is an x86 operating system made in C from scratch that I'm doing to learn. That's it.

It currently has its own bootloader (![JBoot](https://github.com/jlxip/jotadOS/blob/master/src/JBoot/README.md)), kernel, drivers and file system (![JOTAFS](https://github.com/jlxip/jotadOS/blob/master/src/kernel/drivers/storage/FS/JOTAFS/README.md)).

In the future it probably will have its own C library, extremely simple user-space applications, and maybe even a C compiler.

## Compilation
Before you can compile jotadOS, you need the following things:
- `i686-elf-gcc`
- `nasm`
- `qemu` and/or `bochs` and/or `VirtualBox`
- `python`, `genisoimage`
- `bison`, `flex`, `gmp3`, `mpc`, `mpfr`, `texinfo`, `xorriso`, `mtools`

In order to compile `i686-elf-gcc`, open a terminal and do the following (do NOT copy paste):
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

To compile jotadOS, open a new terminal in the directory, and run `./build`.

You can use `./run` to compile and run `qemu`, or `./debug` to compile and run `bochs`.

## TODO list
Here's a list of the big things I've managed to do so far as well as the next steps.

- [X] Simple kernel that writes to the screen (terminal driver), and boots with GRUB.
- [X] GDT.
- [X] IDT.
- [X] Keyboard driver (Spanish layout).
- [X] Paging.
- [X] Kernel panics.
- [X] Simple kernel C library with ![liballoc](https://github.com/blanham/liballoc).
- [X] PCI driver.
- [X] Virtual 8086 mode and VESA video modes.
- [X] Text cursor in video mode.
- [X] ATA PIO driver.
- [X] JBoot, my bootloader, instead of GRUB.
- [X] ATAPI and ISO9660 drivers.
- [X] JOTAFS as the main filesystem.
- [X] Software interrupts and user space.
- [ ] User space C library that can be dinamically linked with the user space binaries.
- [ ] Network driver.
- [ ] TCP.
- [ ] Cryptographic library.
- [ ] Multitasking.
- [ ] FIFOs for inter-process communication.

## Sources
I have used several resources in order to do this project.
- https://osdev.org <-- Mainly this one.
- https://www.youtube.com/channel/UCQdZltW7bh1ta-_nCH7LWYw
- http://ctyme.com/rbrown.htm
- Intel 64 and IA-32 Architectures Software Developer's Manual
