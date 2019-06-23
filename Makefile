all: jotadOS.iso

run: jotadOS.iso
	qemu-system-x86_64 -cdrom jotadOS.iso

debug: jotadOS.iso
	bochs -f bochs.txt

jotadOS.iso: clean bin/boot/grub/grub.cfg bin/boot/jotadOS.bin
	grub-mkrescue -o jotadOS.iso bin

bin/boot/jotadOS.bin: obj/boot.o megaCompile
	i686-elf-gcc -T linker.ld -o bin/boot/jotadOS.bin -ffreestanding -O2 -nostdlib obj/*.o

megaCompile:
	./megaCompile.py

obj/boot.o: src/boot.s
	i686-elf-as src/boot.s -o obj/boot.o

clean:
	rm obj/*.o bin/boot/jotadOS.bin jotadOS.iso jotadOS.iso.lock &> /dev/null || true
