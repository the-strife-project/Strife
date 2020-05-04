OBJPATH=../../obj/kernel
RESULT=../../iso/boot/kernel.bin

CXX=i686-elf-g++
INCLUDES=-I../
CXXFLAGS=$(INCLUDES) -std=c++11 -ffreestanding -O3 -Wall -Wextra -Werror -fno-exceptions -fno-rtti -fno-use-cxa-atexit

ASM=nasm
ASMFLAGS=-f elf32

LINKER=$(CXX)
LINKER_FLAGS=-T linker.ld -std=c++11 -ffreestanding -O3 -nostdlib

CXX_OBJS=$(shell find . -type f -iname '*.cpp' | sed 's/\.cpp/\.o/g' | xargs -I {} echo "$(OBJPATH)/"{})
ASM_OBJS=$(shell find . -type f -iname '*.asm' | sed 's/\.asm/\.o/g' | xargs -I {} echo "$(OBJPATH)/"{})

.PHONY: all

all: $(RESULT)

$(RESULT): $(CXX_OBJS) $(ASM_OBJS)
	$(LINKER) `find $(OBJPATH) -type f -iname '*.o'` -o $@ $(LINKER_FLAGS)
	strip $(RESULT)


-include $(CXX_OBJS:.o=.o.d)

# This line allegedly solve the issue of renaming dependencies.
# Haven't tested it, but might come handy some day.
# @sed -e 's/.*://' -e 's/\\$$//' < $@.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $@.d
# Source: http://scottmcpeak.com/autodepend/autodepend.html

$(OBJPATH)/%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)
	$(CXX) -MM $< -o $@.d.tmp $(CXXFLAGS)
	@sed -e 's|.*:|$@:|' < $@.d.tmp > $@.d
	@rm -f $@.d.tmp

$(OBJPATH)/%.o: %.asm
	$(ASM) $< -o $@ $(ASMFLAGS)
