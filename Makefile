QEMU=qemu-system-x86_64
VHDD=hdd10M.img

.PHONY: all runHDD run debugHDD debug build welcome kernel jboot extras extra_files clean

# Default target, just build the ISO
all: jotaOS.iso

# Target for building the ISO
jotaOS.iso: build
	genisoimage -no-emul-boot -b boot/CD.bin -o jotaOS.iso iso
	@echo -e "\n\e[1;33mIso file created.\e[0m"

# Target for running with a hard drive (twice: CD and HDD)
runHDD: run
	$(QEMU) -hda $(VHDD)

# Target for running the ISO
run: jotaOS.iso
	head -c 10485760 /dev/zero > $(VHDD)
	$(QEMU) -cdrom jotaOS.iso -hda $(VHDD)

# Target for debugging with a hard drive. Run the first time with qemu for a faster process
debugHDD: run
	bochs -f bochs_config/bochs_HDD.txt

# Target for debugging the ISO
debug: jotaOS.iso
	head -c 10485760 /dev/zero > $(VHDD)
	bochs -f bochs_config/bochs.txt

build: welcome iso obj kernel jboot extras extra_files

welcome:
	@echo -e "\e[1;36mCompiling jotaOS...\e[0m\n"

iso:
	mkdir -p iso/boot

# Copies the directory structure from src/ to obj/
obj:
	cd src && find . -type d -exec mkdir -p ../obj/{} \;

kernel:
	cd src/kernel && make

jboot:
	cd src/JBoot && make

extras:
	cd src/extra && make

extra_files:
	cp -rv extra/font iso/font

clean:
	rm -r obj/ iso/ jotaOS.iso jotaOS.iso.lock hdd10M.img &> /dev/null || true
