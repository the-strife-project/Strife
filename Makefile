QEMU=qemu-system-x86_64
VHDD=hdd10M.img
TARGET=jotaOS.iso

.PHONY: all runHDD run debugHDD debug kernel jboot extras extra_files clean

# Default target, just build the ISO
all: $(TARGET)

# Main rule (builds the ISO)
$(TARGET): iso obj kernel jboot extras extra_files
	genisoimage -no-emul-boot -b boot/CD.bin -o $(TARGET) iso

# Target for running with a hard drive (twice: CD and HDD)
runHDD: run
	$(QEMU) -drive file=$(VHDD),format=raw

# Target for running the ISO
run: $(TARGET)
	head -c 10485760 /dev/zero > $(VHDD)
	$(QEMU) -cdrom $(TARGET) -drive file=$(VHDD),format=raw

# Target for debugging with a hard drive. Run the first time with qemu for a faster process
debugHDD: run
	bochs -f bochs_config/bochs_HDD.txt

# Target for debugging the ISO
debug: $(TARGET)
	head -c 10485760 /dev/zero > $(VHDD)
	bochs -f bochs_config/bochs.txt

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
	cd extra && make

clean:
	rm -r obj/ iso/ $(TARGET) $(TARGET).lock hdd10M.img &> /dev/null || true
