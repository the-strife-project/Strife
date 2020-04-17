#ifdef STL_TEST

#include <klibc/STL/stack>
#include <klibc/stdio.hpp>

int stack_test() {
	stack<int> s;
	for(int i=0; i<10; i++) s.push(i);
	stack<int> s2 = s;
	if(s != s2) return 1;
	if(s.size() != 10) return 2;

	for(int i=9; i; i--) {
		if(s.top() != i) return 3;
		s.pop();
	}

	if(s == s2) return 4;

	return 0;
}

#endif
