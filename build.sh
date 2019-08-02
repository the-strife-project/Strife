#!/bin/bash

# Check whether the 'obj' directory exists.
if [ ! -d obj ]; then
	mkdir obj
fi
if [ ! -d iso ]; then
	mkdir iso
fi

_compileGCC () {
	toRun="i686-elf-gcc -c -I./src
		-std=gnu99 -ffreestanding -O2 -Wall -Wextra
		$0 -o obj/$(basename $0 .c).o"
	$($toRun)
	if [ $? != 0 ]; then
		echo
		echo -e "\e[32m$toRun\e[0m"
		echo -e "\e[1;31mBUILD FAILED!\e[0m"
		return 1
	fi
}
export -f _compileGCC

_compileASM () {
	toRun="nasm $0 -o obj/$(basename $0 .asm).o -f elf32"
	$($toRun)
	if [ $? != 0 ]; then
		echo
		echo -e "\e[32m$toRun\e[0m"
		echo -e "\e[1;31mBUILD FAILED!\e[0m"
		return 1
	fi
}
export -f _compileASM

# Welcome!
echo -e "\e[1;36mCompiling jotadOS...\e[0m"

# Kernel (C)
find src/kernel -type f -iname *.c | xargs -n1 bash -c '_compileGCC $1 || exit 255'
echo -e "\e[1;33mKernel C files done.\e[0m"

# Kernel (asm)
find src/kernel -type f -iname *.asm | xargs -n1 bash -c '_compileASM $1 || exit 255'
find src/ -iname boot.asm | xargs -n1 bash -c '_compileASM $1 || exit 255'
echo -e "\e[1;33mKernel asm files done.\e[0m"

# Libc
find src/libc -type f -iname *.c | xargs -n1 bash -c '_compileGCC $1 || exit 255'
echo -e "\e[1;33mLibc done.\e[0m"

# The whole kernel
i686-elf-gcc -T linker.ld -I./src -std=gnu99 -ffreestanding -O2 -nostdlib obj/*.o -o iso/kernel.bin
