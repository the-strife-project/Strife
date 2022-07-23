IMG := Strife.iso
IMGPATH := img
SHELL := /bin/bash
BOOT := $(IMGPATH)/boot
BIN := $(IMGPATH)/bin
LIBSPATH := $(IMGPATH)/libs
export STRIFE_LIBS := $(shell pwd)/$(LIBSPATH)
export STRIFE_STDLIB_HEADERS := $(shell pwd)/projects/stdlib/pubheaders
export STRIFE_HELPER := $(shell pwd)/projects/helper

MAKEFLAGS += -s

LIMINE_PATH := limine
_LIMINE_FILES := limine-cd.bin limine.sys
LIMINE_FILES := $(_LIMINE_FILES:%=$(LIMINE_PATH)/%)

.PHONY: run debug all $(IMG) clean

all: $(IMG)
run: all
	@qemu-system-x86_64 -cdrom $(IMG) -cpu IvyBridge -machine q35
	@#qemu-system-x86_64 -cdrom $(IMG) -cpu Broadwell -machine q35 -m 128M

pretty = "\e[34m\e[1m--- "$(1)" ---\e[0m"
$(IMG): limine/limine-deploy
	@echo -e $(call pretty,LIBRARIES)
	@$(MAKE) libs --no-print-directory	# -j makes no difference

	@echo -e $(call pretty,PROJECTS)
	@$(MAKE) programs -j`nproc` --no-print-directory

	@echo -e $(call pretty,$(IMG))
	@cp -v limine.cfg $(LIMINE_FILES) $(BOOT)/
	@xorriso -as mkisofs -b boot/limine-cd.bin -no-emul-boot \
		-boot-load-size 4 -boot-info-table \
		--protective-msdos-label -o $@ $(IMGPATH) &> /dev/null
	@limine/limine-deploy $(IMG)

	@echo -e "\n\e[34m\e[1mThank you for using Strife\e[0m"

limine/limine-deploy: limine/limine-deploy.c
	@$(MAKE) -C limine/


-include projects/libs.txt
-include projects/programs.txt

# Always compile
.PHONY: libs programs clean $(PROJECTS)


# Order is critical
libs: | $(LIBSPATH)
	$(foreach lib, $(LIBS), @$(MAKE) -C projects/$(lib) -j`nproc`; \
		cp -v projects/$(lib)/$($(lib)) $(LIBSPATH)/;)
$(LIBSPATH):
	@mkdir -p $@


# Order doesn't matter
programs: $(IMGBOOT) $(IMGBIN)
$(IMGBOOT): | $(BOOT)
	@$(MAKE) -C projects/$@	# Do not use -j!
	@cp -v projects/$@/$($@) $(BOOT)/
$(IMGBIN): | $(BIN)
	@$(MAKE) -C projects/$@
	@cp -v projects/$@/$($@) $(BIN)/


$(BOOT):
	@mkdir -p $@
$(BIN):
	@mkdir -p $@


clean:
	@echo '🧹🧹🧹'
	@$(foreach x, $(LIBS) $(IMGBOOT) $(IMGBIN), $(MAKE) -C projects/$(x) clean;)
	rm -rf $(IMG) $(IMGPATH)
