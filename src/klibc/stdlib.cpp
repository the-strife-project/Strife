#include <klibc/stdlib.h>

void* operator new(size_t sz) { return jmalloc(sz); }
void* operator new[](size_t sz) { return jmalloc(sz); }
void operator delete(void* p) { jfree(p); }
void operator delete[](void* p) { jfree(p); }
