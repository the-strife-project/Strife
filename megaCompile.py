#!/usr/bin/env python2.7
# Do not run this script. This is called by the Makefile.
import os

OBJ = 'obj'
BASE = '-c -Iinclude -std=gnu99 -ffreestanding -O2 -Wall -Wextra'

# Modes: False (assembly), True (C)
def compile(path, prefix='', mode=True):
	ret = []

	files = os.listdir(path)
	for i in files:
		if mode:
			if i.split('.')[1] != 'c': continue
		else:
			if i.split('.')[1] != 's': continue

		file = path + '/' + i
		ofile = OBJ + '/' + prefix + i.split('.')[0] + '.o'
		if mode:
			toExecute = 'i686-elf-gcc ' + file + ' -o ' + ofile + ' ' + BASE
		else:
			toExecute = 'i686-elf-as ' + file + ' -o ' + ofile
		ret.append(toExecute)

	return ret

if __name__ == '__main__':
	toCompile = compile('src/kernel')
	toCompile += compile('src/kernel', mode=False)
	toCompile += compile('src/libc/string', 'string_')
	toCompile += compile('src/libc/stdio', 'stdio_')

	for i in toCompile:
		print i
		os.system(i)
