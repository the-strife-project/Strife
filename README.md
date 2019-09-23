# jotadOS

## Introduction
**MOST OF THE FUNCTIONALITY LISTED HERE IS NOT IMPLEMENTED YET**

jotadOS is an x86 operating system made in C from scratch based on the idea of compartmentalization.

It has its own bootloader (JBoot), kernel, file system (JOTAFS), and user-space applications.

## The jotadOS philosophy
*(This philosophy is, as the implementation, licensed with the GNU GPL 3 license. See the LICENSE file in this directory.)*

The idea behind jotadOS is the following:

### Syscalls
First, there is the kernel, which handles every request from the applications. An app probably will require low-level functionality (such as accessing the network), so it has to make a system call (henceforth, "syscall"). These are listed when installing an application, and the user can switch them on and off every time they want. This prevents apps from accessing a functionality they are not supposed to (such as the before-mentioned network).

### Compartmentalization
Then, there's the main application, "MSS" (main system shell), which offers the command line input and communicates with the person sitting on the chair.

All applications are compartmentalized, which means that none of them have direct access to each other. The files are not stored in a regular filesystem like most operating systems out there. Instead, each program has its own files in a protected space of the hard disk (which can perfectly be encrypted natively).

That means that an application APP1 (e.g. a file sharer) cannot access directly the files created by APP2 (e.g. a text editor). Instead, it has to follow the jotadOS file access flux.

### The jotadOS file access flux (JFAF)
When APP1 requires a file from APP2 (or a file list), the following protocol takes place:

*(This will have a neat graph here.)*

1. APP1 asks the kernel for APP2:examplefolder:test.txt (note that ':' is a separator).
2. The kernel asks MSS for permission (see JFAF permissions).
3. MSS returns to the kernel YES or NO.
4. The kernel stores in its space information about the request and the response.
- If MSS denies the access, APP1 is informed, and the flux terminates.
- If MSS allows it, the flux continues.
5. The kernel accesses APP2's storage, and gets the file.
- If the file is encrypted, the following happens.
- 1. The kernel asks MSS for the key.
- 2. MSS returns the key to the kernel.
- 3. The kernel decrypts the file, and the flux continues.
- - If the key is incorrect, it will try one more time, and then give up, informing APP1 about the situation.
- If the file isn't, the flux continues.
6. The kernel sends the (decrypted, or not encrypted in the first place) file to APP1 and the flux ends.

### JFAF permissions
MSS, when requested permission to access an APP2's file by APP1, must provide, at least, the following options:
* Yes.
* No.
* (Yes) Always this file.
* (Yes) Always all files (from APP2).
* (Yes) Always some files (explained below).
* (No) Never this file.
* (No) Never any file (from APP2).
* (No) Never some files (same).

The options "Always some files" and "Never some files" allows the user to segmentate folders in the app's storage. For instance, it can allow any file in the folder "college:essays", but none of the files in the folders "college:homework" or "books".

This information can be stored in MSS' files, but when the kernel asks for permission, only YES or NO can be returned.

## Dependencies
In order to compile jotadOS, you need the following things:
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
make install-target-libgcc	# This might fail. Ignore it in that case.
```

Now write `PATH="$HOME/opt/cross/bin:$PATH"` at the end of your shell profile (.bashrc, .zshrc...)

## Compilation
To compile jotadOS, open a new terminal in the directory, and run `./build`.

You can use `./run` to compile and run `qemu`, or `./debug` to compile and run `bochs`.

## Sources
I have used several sources in order to do this project (ordered by importance).
- https://osdev.org <-- Mainly this one.
- https://www.youtube.com/channel/UCQdZltW7bh1ta-_nCH7LWYw
- https://github.com/mkilgore/protura
- https://github.com/AlexandreRouma/LimeOS
