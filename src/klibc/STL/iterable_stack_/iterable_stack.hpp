#ifndef ITERABLE_STACK_H
#define ITERABLE_STACK_H

#include <klibc/STL/stack>

template<typename T> class iterable_stack : public stack<T> {
public:
	typedef typename list<T>::iterator iterator;
	typedef typename list<T>::const_iterator const_iterator;

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;
};

#include <klibc/STL/iterable_stack_/iterable_stack_imp.hpp>

#endif
