#ifndef STDLIB_H
#define STDLIB_H

#include <kernel/klibc/stdlib/pool-alloc/pool-alloc.hpp>
#include <string>

void* operator new(size_t sz);
void* operator new[](size_t sz);
void operator delete(void* p);
void operator delete[](void* p);

void memcpy(void* dst, void* src, size_t sz);
void memclear(void* dst, size_t sz);

string itoa(int n);
int atoi(string str);

string htoa(uint32_t n);
int htoi(string str);

#endif
