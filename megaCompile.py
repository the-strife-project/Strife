#!/usr/bin/env python2.7
# Do not run this script. This is called by the Makefile.
import os

OBJ = 'obj'
BASE = '-c -Iinclude -std=gnu99 -ffreestanding -O2 -Wall -Wextra'

def compile(path, prefix=''):
	ret = []

	files = os.listdir(path)
	for i in files:
		if i.split('.')[1] != 'c': continue
		cfile = path + '/' + i
		ofile = OBJ + '/' + prefix + i.split('.')[0] + '.o'
		toExecute = 'i686-elf-gcc ' + cfile + ' -o ' + ofile + ' ' + BASE
		ret.append(toExecute)

	return ret

if __name__ == '__main__':
	toCompile = compile('src/kernel')
	toCompile += compile('src/libc/string', 'string_')
	toCompile += compile('src/libc/stdio', 'stdio_')

	for i in toCompile:
		print i
		os.system(i)
