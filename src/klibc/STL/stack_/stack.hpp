#ifndef STACK_H
#define STACK_H

#include <common/types.hpp>
#include <klibc/STL/list>

template<typename T> class stack {
protected:	// Protected for "iterable_stack".
	list<T> data;

public:
	size_t size() const;

	void push(T e);
	T& top();
	const T& top() const;
	void pop();

	bool operator==(const stack<T>& other) const;
	bool operator!=(const stack<T>& other) const;
};

#include <klibc/STL/stack_/stack_imp.hpp>

#endif
