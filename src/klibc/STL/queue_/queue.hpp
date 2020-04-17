#ifndef QUEUE_H
#define QUEUE_H

#include <common/types.hpp>
#include <klibc/STL/list>

template<typename T> class queue {
private:
	list<T> data;

public:
	size_t size() const;

	void push(T e);
	T& front();
	const T& front() const;
	T& back();
	const T& back() const;
	void pop();

	bool operator==(const queue<T>& other) const;
	bool operator!=(const queue<T>& other) const;
};

#include <klibc/STL/queue_/queue_imp.hpp>

#endif
