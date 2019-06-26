#!/bin/bash
./clean.sh
cmake .
make
sudo cp bin/boot/jotadOS.bin /boot/jotadOS.bin
