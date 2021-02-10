IMG := img.hdd
TMPDIR := tmp

IMGPATH := img
BOOT := $(IMGPATH)/boot

LIMINE := limine/limine-install

# I don't think I'm able to stress how much I hate the mounting/unmount of img.hdd. I'll improve this soon

.PHONY: run debug all clean

all: $(IMG)
run: all
	qemu-system-x86_64 -hda $(IMG)
debug: all
	bochs -f bochs_config/bochs.txt

-include projects/projects.txt
_BOOTFILES := limine.cfg $(foreach x,$(PROJECTS),$($(x)))
BOOTFILES := $(_BOOTFILES:%=$(BOOT)/%)

$(IMG): $(BOOT) $(BOOTFILES) $(LIMINE)
	@echo -e "\n\n--- Creating $(IMG) now ---\n"
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

$(BOOT):
	mkdir -p $(BOOT)

$(BOOT)/limine.cfg: limine.cfg
	@cp -v $< $@

noext = $(firstword $(subst ., ,$(1)))
.SECONDEXPANSION:
$(BOOT)/%: projects/$$(call noext,$$*)/%
	@cp -v $< $@
projects/%:
	$(MAKE) -C projects/$(firstword $(subst /, ,$*))

$(LIMINE): limine/limine-install.c
	$(MAKE) -C limine limine-install

clean:
	$(MAKE) -C limine clean
	$(foreach x,$(PROJECTS),$(MAKE) -C projects/$(x) clean)
	rm -rf $(IMG) $(IMGPATH)
