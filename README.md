# jotadOS

## Dependencies
In order to compile jotadOS, you need the following things:
- `i686-elf-gcc`
- `i686-elf-as`
- `qemu` and/or `bochs` (optional)
- `cmake` (and `make`)
- `bison`, `flex`, `gmp3`, `mpc`, `mpfr`, `texinfo`, `grub2`, `xorriso`

The last two set of tools can be obtained via your package manager.

In order to compile `i686-elf-gcc` and `i686-elf-as`, open a terminal and do the following (do NOT copy paste):
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
make all-gcc -j8
make all-target-libgcc -j8
make install-gcc
make install-target-libgcc	# This might fail. Ignore it in that case.
```

Now write `PATH="$HOME/opt/cross/bin:$PATH"` at the end of your shell profile (.bashrc, .zshrc...)

## Compilation
To compile jotadOS, open a new terminal in the directory, and run `./build.sh`.

You can use `./run.sh` to compile and run `qemu`, or `./debug.sh` to compile and run `bochs`.

## Sources
I have used several sources in order to do this project.
- https://osdev.org
- https://www.youtube.com/channel/UCQdZltW7bh1ta-_nCH7LWYw
- https://github.com/mkilgore/protura
- https://github.com/AlexandreRouma/LimeOS
