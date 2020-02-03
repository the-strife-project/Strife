#ifndef WINDOW_H
#define WINDOW_H

#include <klibc/STL/iterable_stack>

template<typename T> class window {
private:
	iterable_stack<T> left_st, right_st;

public:
	window() = default;
	window(const window& other) = default;
	window(window&& other) = default;

	size_t size() const;

	T& get();
	const T& get() const;

	void put(const T& e);
	void remove();
	void clear();

	void left();
	void right();

	size_t nleft() const;
	size_t nright() const;
	bool isBeginning() const;
	bool isFirst() const;
	bool isLast() const;

	window<T>& operator=(const window<T>& other) = default;
	window<T>& operator=(window<T>&& other) = default;
	window<T>& operator++();	// right();
	window<T> operator++(int);
	window<T>& operator--();	// left();
	window<T> operator--(int);

	typedef typename iterable_stack<T>::iterator iterator;
	typedef typename iterable_stack<T>::const_iterator const_iterator;

	/*
		begin() is not actually the beginning.
		The iterator of window is exactly the iterator of right_st.
		So, *begin() is right_st.top().
		Kind of weird, I know. It's what I need for the keyboard.
	*/
	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
};

#include <klibc/STL/window_/window_imp.hpp>

#endif
