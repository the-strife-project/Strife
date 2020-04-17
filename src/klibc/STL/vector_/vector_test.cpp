#ifdef STL_TEST

#include <klibc/STL/vector>
#include <klibc/stdio.hpp>

int vector_test() {
	vector<int> v;
	v.push_back(0);
	if(v.size() != 1) return 1;

	vector<int> v2 = v;
	if(v != v2) return 2;

	for(int i=1; i<=10; i++) v.push_back(i);
	if(v.size() != 11) return 3;

	v.push_front(99);

	if(v[0] != v.front()) return 4;
	if(v.front() != 99) return 5;
	v.pop_front();
	if(v.front() != 0) return 6;
	v.pop_back();
	if(v.back() != 9) return 7;

	for(size_t i=0; i<v.size(); i++) {
		if(v[i] != (int)i) return 8;
	}

	int counter = 0;
	for(auto& x : v) {
		if(x != counter) return 9;
		counter++;
	}

	counter = 0;
	for(auto const& x : v) {
		if(x != counter) return 10;
		counter++;
	}

	return 0;
}

#endif
