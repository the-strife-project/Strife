#ifndef VECTOR_H
#define VECTOR_H

#include <common/types.hpp>

template<typename T> class vector {
private:
	T* data;
	size_t sz;
	size_t allocated;

	void more();

public:
	vector();
	vector(const vector<T>& other);
	vector(vector<T>&& other);
	~vector();

	size_t size() const;
	void disp_right(size_t idx, size_t count);
	void disp_left(size_t idx, size_t count);

	// Back.
	void push_back(T e);
	void push_back(const vector<T>& other);
	void pop_back();
	T& back();
	const T& back() const;

	// Front.
	void push_front(T e);
	void push_front(const vector<T>& other);
	void pop_front();
	T& front();
	const T& front() const;

	// General operations.
	void invert();
	void clear();

	// Operators.
	vector<T>& operator=(const vector<T>& other);
	vector<T>& operator=(vector<T>&& other);
	T& operator[](size_t idx);
	const T& operator[](size_t idx) const;
	bool operator==(const vector<T>& other) const;
	bool operator!=(const vector<T>& other) const;
	vector<T>& operator+=(const vector<T>& other);

	// Iterators.
	typedef T* iterator;
	typedef const T* const_iterator;

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;
};

#include <klibc/STL/vector_/vector_imp.hpp>

#endif
