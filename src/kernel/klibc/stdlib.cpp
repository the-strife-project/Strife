#include <kernel/klibc/stdlib.hpp>

void* operator new(size_t sz) {
	return jmalloc(sz);
}

void* operator new[](size_t sz) {
	return jmalloc(sz);
}

inline void generalDelete(void* p) {
#ifdef STDLIB_DEBUG
	bool found = false;
	for(size_t i=0; i<ctr; ++i) {
		if(debug[i] == p) {
			debug[i] = nullptr;
			found = true;
			break;
		}
	}
	if(!found)
		while(true);
	jfree(p);
#else
	jfree(p);
#endif
}

void operator delete(void* p) {
	return jfree(p);
}

void operator delete[](void* p) {
	return jfree(p);
}

// TODO: use Duff's drive.
void memcpy(void* dst, void* src, size_t sz) {
	// TODO: Check if both dst and src are aligned. Change MPI/messages.cpp when done.
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

void memclear(void* dst, size_t sz) {
	// Really slow. I mean, the slowest possible. I'll improve this soon, don't dwell on it.
	uint8_t* i = (uint8_t*)dst;
	while(sz--)
		*(i++) = 0;
}
