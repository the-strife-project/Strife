QEMU=qemu-system-x86_64
VHDD=hdd10M.img

.PHONY: runHDD run debugHDD debug build welcome compile_kernel compile_klibc link_kernel compile_jboot compile_extras copy_extra clean

# Target for building the ISO
jotaOS.iso: build
	genisoimage -no-emul-boot -b boot/CD.bin -o jotaOS.iso iso
	@echo -e "\e[1;33mIso file created.\e[0m"

# Target for running with a hard drive (twice: CD and HDD)
runHDD: run
	$(QEMU) -hda $(VHDD)

# Target for running the ISO
run: jotaOS.iso
	head -c 10485760 /dev/zero > $(VHDD)
	$(QEMU) -cdrom jotaOS.iso -hda $(VHDD)

# Target for debugging with a hard drive. Run the first time with qemu for faster process.
debugHDD: run
	bochs -f bochs_HDD.txt

# Target for debugging the ISO
debug: jotaOS.iso
	head -c 10485760 /dev/zero > $(VHDD)
	bochs -f bochs.txt

build: welcome iso obj compile_kernel compile_klibc link_kernel compile_jboot compile_extras copy_extra

welcome:
	@echo -e "\e[1;36mCompiling jotaOS...\e[0m"

# Creates the 'iso' directory. That's it.
iso:
	mkdir -p iso/boot

# Copies the directory structure from src/ to obj/
obj:
	cd src && find . -type d -exec mkdir -p ../obj/{} \;

compile_kernel:
	cd src/kernel && make

# TODO: Move all of this to the Makefile in "kernel". That implies moving "klibc" to "kernel/klibc". Will be done in the exact next commit. I don't want to make a huge commit for this.
compile_klibc:
	cd src/klibc && make

# TODO: Will also have to move "boot.asm" inside "kernel". I want this to be modular, which means easy to work with. I do know the target below is horrible.
link_kernel:
	nasm src/boot.asm -o obj/boot.o -f elf32
	i686-elf-g++ -T linker.ld -I./src -std=c++11 -ffreestanding -O2 -nostdlib `find obj -type f -iname '*.o'` -o iso/boot/kernel.bin

compile_jboot:
	cd src/JBoot && make

compile_extras:
	cd src/extra && make

# For copying extra files
copy_extra:
	cp -r extra/font iso/font

clean:
	rm -r obj/ iso/ jotaOS.iso jotaOS.iso.lock hdd10M.img &> /dev/null || true
