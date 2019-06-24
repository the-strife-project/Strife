#ifndef STDLIB_H
#define STDLIB_H

#ifndef _LIBALLOC_H
#include <libc/liballoc.h>
#endif

#ifndef _STRING_H
#include <libc/string.h>
#endif

char* itoa(int n);
int atoi(char* str);

char* htoa(uint32_t n);
int htoi(char* str);

#endif
