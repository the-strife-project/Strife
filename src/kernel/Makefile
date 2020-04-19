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

.PHONY: link build build_cxx_objs build_asm_objs

link: build
	$(LINKER) `find $(OBJPATH) -type f -iname '*.o'` -o $(RESULT) $(LINKER_FLAGS)

build: build_cxx_objs build_asm_objs

build_cxx_objs: $(CXX_OBJS)
build_asm_objs: $(ASM_OBJS)

$(OBJPATH)/%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(OBJPATH)/%.o: %.asm
	$(ASM) $< -o $@ $(ASMFLAGS)
