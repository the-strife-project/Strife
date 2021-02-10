IMG=img.hdd
TMPDIR=tmp

IMGPATH=img
BOOT=$(IMGPATH)/boot

LIMINE=limine/limine-install

# I don't think I'm able to stress how much I hate the mounting/unmount of img.hdd. I'll change this very soon
# Also, this Makefile in general isn't general at all. Will change that even sooner

.PHONY: run debug all files clean

run: all
	qemu-system-x86_64 -hda $(IMG)

debug: all
	bochs -f bochs_config/bochs.txt

all: $(IMG)

$(IMG): files $(LIMINE)
	dd if=/dev/zero bs=1M count=0 seek=64 of=$@
	parted -s $@ mklabel gpt
	parted -s $@ mkpart primary 2048s 100%

	mkdir tmp
	sudo losetup -Pf --show $(IMG) > loopback_dev
	sudo partprobe `cat loopback_dev`
	sudo mkfs.fat -F32 `cat loopback_dev`p1
	sudo mount `cat loopback_dev`p1 $(TMPDIR)

	sudo cp -rv img/* $(TMPDIR)
	sync

	sudo umount $(TMPDIR)
	sudo losetup -d `cat loopback_dev`
	rmdir $(TMPDIR)
	rm -f loopback_dev

	$(LIMINE) $(IMG)

files: $(BOOT) projects/.done $(BOOT)/limine.cfg $(BOOT)/kernel.elf

$(BOOT):
	mkdir -p $(BOOT)

projects/.done:
	$(MAKE) -C projects

$(BOOT)/limine.cfg: limine.cfg
	cp -v $< $@

$(BOOT)/kernel.elf: projects/kernel/kernel.elf
	cp -v $< $@

$(LIMINE): limine/limine-install.c
	$(MAKE) -C limine limine-install

clean:
	$(MAKE) -C limine clean
	$(MAKE) -C projects clean
	rm -rf $(IMG) $(IMGPATH)
