#include <klibc/stdlib.hpp>

void* operator new(size_t sz) { return jmalloc(sz); }
void* operator new[](size_t sz) { return jmalloc(sz); }
void operator delete(void* p) { jfree(p); }
void operator delete[](void* p) { jfree(p); }

/*
	This functions moves memory in the most efficient way I know.
	As we're in a x86 environment, it uses the size of the register
	to move 4 bytes at a time in the first stage, and then move
	the remaining bytes individually.

	Is it an overkill? I don't know.
	This function will be called when moving big chunks of memory,
	so this at least is as fast as moving single characters.
*/
void memcpy(void* dst, void* src, size_t sz) {
	union u {
		void* v;
		size_t* b;
		uint8_t* l;
	};

	u u_src, u_dst;
	u_src.v = src; u_dst.v = dst;

	size_t sz_round = sz >> 2;
	while(sz_round--)
		*(u_dst.b++) = *(u_src.b++);

	sz_round = sz & 0b11;
	while(sz_round--)
		*(u_dst.l++) = *(u_src.l++);
}
