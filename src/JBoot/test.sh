#!/bin/bash
nasm jboot.asm -o bin/jboot.bin && genisoimage -no-emul-boot -b jboot.bin -o jboot.iso bin && bochs -f bochs.txt
