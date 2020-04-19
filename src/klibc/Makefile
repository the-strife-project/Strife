OBJPATH=../../obj/klibc

CXX=i686-elf-g++
INCLUDES=-I../
CXXFLAGS=$(INCLUDES) -std=c++11 -ffreestanding -O3 -Wall -Wextra -Werror -fno-exceptions -fno-rtti -fno-use-cxa-atexit

CXX_OBJS=$(shell find . -type f -iname '*.cpp' | sed 's/\.cpp/\.o/g' | xargs -I {} echo "$(OBJPATH)/"{})

.PHONY: build build_cxx_objs finished

build: build_cxx_objs finished

build_cxx_objs: $(CXX_OBJS)

$(OBJPATH)/%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

finished:
	@echo -e "\e[1;33mKlibc done.\e[0m"
