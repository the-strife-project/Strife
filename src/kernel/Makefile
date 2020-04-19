OBJPATH=../../obj/kernel

CXX=i686-elf-g++
INCLUDES=-I../
CXXFLAGS=$(INCLUDES) -std=c++11 -ffreestanding -O3 -Wall -Wextra -Werror -fno-exceptions -fno-rtti -fno-use-cxa-atexit

ASM=nasm
ASMFLAGS=-f elf32

CXX_OBJS=$(shell find . -type f -iname '*.cpp' | sed 's/\.cpp/\.o/g' | xargs -I {} echo "$(OBJPATH)/"{})
ASM_OBJS=$(shell find . -type f -iname '*.asm' | sed 's/\.asm/\.o/g' | xargs -I {} echo "$(OBJPATH)/"{})

.PHONY: build build_cxx_objs build_asm_objs finished

build: build_cxx_objs build_asm_objs finished

build_cxx_objs: $(CXX_OBJS)
build_asm_objs: $(ASM_OBJS)

$(OBJPATH)/%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(OBJPATH)/%.o: %.asm
	$(ASM) $< -o $@ $(ASMFLAGS)

finished:
	@echo -e "\e[1;33mKernel files done.\e[0m"
