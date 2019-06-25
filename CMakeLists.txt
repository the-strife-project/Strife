cmake_minimum_required(VERSION 3.0)

project(jotadOS)

enable_language(ASM)

# Remove the goddamn '-rdynamic'.
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS " ")

set(CMAKE_C_COMPILER "i686-elf-gcc")
set(CMAKE_ASM_COMPILER "i686-elf-as")
set(CMAKE_LINKER "i686-elf-gcc")

set(CMAKE_C_FLAGS "-c -I./src -std=gnu99 -ffreestanding -O2 -Wall -Wextra ${CMAKE_C_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "-T linker.ld -o bin/boot/jotadOS.bin -ffreestanding -O2 -nostdlib ${CMAKE_EXE_LINKER_FLAGS}")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> ${CMAKE_EXE_LINKER_FLAGS} <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

include_directories(jotadOS "./")

# Boot
file(GLOB SRC_BOOT "src/boot.s")

# Kernel
file(GLOB SRC_KERNEL "src/kernel/*.s" "src/kernel/*.c")
# Kernel - Drivers
file(GLOB SRC_KERNEL_DRIVERS "src/kernel/drivers/*.s" "src/kernel/drivers/*.c")
# LibC - Stdio
file(GLOB SRC_LIBC_STDIO "src/libc/stdio/*.c")
# LibC - String
file(GLOB SRC_LIBC_STRING "src/libc/string/*.c")
# LibC - Math
file(GLOB SRC_LIBC_MATH "src/libc/math/*.c")
# LibC - Stdlib
file(GLOB SRC_LIBC_STDLIB "src/libc/stdlib/*.c")

set(SOURCES ${SRC_BOOT} ${SRC_KERNEL}
	${SRC_KERNEL_DRIVERS}
	${SRC_LIBC_STDIO} ${SRC_LIBC_STRING} ${SRC_LIBC_MATH} ${SRC_LIBC_STDLIB}
)

add_executable(jotadOS ${SOURCES})

set_target_properties(jotadOS PROPERTIES OUTPUT_NAME "bin/boot/jotadOS.bin")
add_custom_target(iso COMMAND grub-mkrescue -o jotadOS.iso bin)
add_custom_target(run COMMAND qemu-system-x86_64 -cdrom jotadOS.iso)
add_custom_target(debug COMMAND bochs -f bochs.txt)

add_dependencies(iso jotadOS)
add_dependencies(run iso)
add_dependencies(debug iso)
