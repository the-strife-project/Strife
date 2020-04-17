#ifdef STL_TEST

#include <klibc/STL/queue>
#include <klibc/stdio.hpp>

int queue_test() {
	queue<int> q;
	for(int i=0; i<10; i++) q.push(i);
	queue<int> q2 = q;
	if(q != q2) return 1;
	if(q.size() != 10) return 2;

	for(int i=0; i<10; i++) {
		if(q.front() != i) return 3;
		q.pop();
	}

	if(q == q2) return 4;

	return 0;
}

#endif
