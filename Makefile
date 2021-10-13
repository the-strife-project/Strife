IMG := jotaOS.iso
TMPDIR := tmp

IMGPATH := img
BOOT := $(IMGPATH)/boot
LIBSPATH := $(IMGPATH)/libs
export JOTAOS_LIBS := $(shell pwd)/$(LIBSPATH)
export JOTAOS_STDLIB_HEADERS := $(shell pwd)/projects/stdlib/pubheaders

LIMINE_PATH := limine
_LIMINE_FILES := limine-cd.bin limine.sys
LIMINE_FILES := $(_LIMINE_FILES:%=$(LIMINE_PATH)/%)

.PHONY: run debug all $(IMG) clean

all: $(IMG)
run: all
	qemu-system-x86_64 -cdrom $(IMG) -cpu IvyBridge
debug: all
	bochs -f bochs_config/bochs.txt

pretty = "\e[34m\e[1m--- "$(1)" ---\e[0m"
$(IMG):
	@echo -e $(call pretty,LIBRARIES)
	@$(MAKE) libs --no-print-directory	# -j makes no difference

	@echo -e $(call pretty,PROGRAMS)
	@$(MAKE) programs -j`nproc` --no-print-directory

	@echo -e $(call pretty,$(IMG))
	@cp -v limine.cfg $(LIMINE_FILES) $(BOOT)/
	@genisoimage -no-emul-boot -b boot/limine-cd.bin -boot-load-size 4 -boot-info-table -o $@ $(IMGPATH) 2> /dev/null


-include projects/libs.txt
-include projects/programs.txt

# Always compile
.PHONY: libs programs clean $(PROGRAMS)


# Order is critical
libs: | $(LIBSPATH)
	$(foreach lib, $(LIBS), @$(MAKE) -C projects/$(lib) -j`nproc`; \
		cp -v projects/$(lib)/$($(lib)) $(LIBSPATH)/$($(lib)))
$(LIBSPATH):
	@mkdir -p $@


# Order doesn't matter
programs: $(PROGRAMS)
$(PROGRAMS): | $(BOOT)
	@$(MAKE) -C projects/$@	# Do not use -j!
	@cp -v projects/$@/$($@) $(BOOT)/$($@)
$(BOOT):
	@mkdir -p $@


clean:
	@$(foreach x, $(LIBS) $(PROGRAMS), $(MAKE) -C projects/$(x) clean;)
	rm -rf $(IMG) $(IMGPATH)
