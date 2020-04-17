#ifdef STL_TEST

#include <klibc/STL/list>
#include <klibc/stdio.hpp>

int list_test() {
	list<int> l;
	l.push_back(0);
	if(l.size() != 1) return 1;

	list<int> l2 = l;
	if(l != l2) return 2;

	for(int i=1; i<=10; i++) l.push_back(i);
	if(l.size() != 11) return 3;

	l.push_front(99);

	if(l.front() != 99) return 4;
	l.pop_front();
	if(l.front() != 0) return 5;

	int counter = 0;
	for(auto& x : l) {
		if(x != counter) return 6;
		counter++;
	}

	counter = 0;
	for(auto const& x : l) {
		if(x != counter) return 7;
		counter++;
	}

	for(int i=0; i<10; i++) {
		if(l.front() != i) return 8;
		l.pop_front();
	}

	return 0;
}

#endif
