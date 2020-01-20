#ifndef VECTOR_H
#define VECTOR_H

#include <common/types.h>

template<typename T> class vector {
private:
	T* data;
	size_t sz;
	size_t allocated;

	void more();
	void disp_right(size_t idx, size_t count);
	void disp_left(size_t idx, size_t count);

public:
	vector();
	vector(const vector<T>& other);
	~vector();

	size_t size() const;

	// Back.
	void push_back(T e);
	void pop_back();
	T& back();
	const T& back() const;

	// Front.
	void push_front(T e);
	void pop_front();
	T& front();
	const T& front() const;

	// Operators.
	vector<T>& operator=(const vector<T>& other);
	T& operator[](size_t idx);
	const T& operator[](size_t idx) const;
	bool operator==(const vector<T>& other) const;
	bool operator!=(const vector<T>& other) const;

	// Iterators.
	typedef T* iterator;
	typedef const T* const_iterator;

	iterator begin();
	iterator end();
	const_iterator cbegin() const;
	const_iterator cend() const;
};

#include <klibc/STL/vector_/vector_imp.h>

#endif
