#ifndef DLIST_H
#define DLIST_H

#include <common/types.hpp>

/*
	This datatype has not been deeply tested and as a result might have bugs.
	Specially the 'insert' and 'remove' methods.
*/

template<typename T> class dlist {
private:
	struct node {
		T data;
		node* prev;
		node* next;
	};

	node* first;
	node* last;
	size_t sz;

public:
	dlist();
	dlist(const dlist& other);
	dlist(dlist&& other);
	~dlist();

	size_t size() const;
	void clear();

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
	dlist<T>& operator=(const dlist<T>& other);
	dlist<T>& operator=(dlist<T>&& other);
	bool operator==(const dlist<T>& other) const;
	bool operator!=(const dlist<T>& other) const;

	// Iterators.
	class iterator {
	private:
		node* current;
		iterator(node* current);
		friend class dlist;

	public:
		iterator();
		iterator(const iterator& other);
		iterator& operator=(const iterator& other);
		bool operator==(const iterator& other) const;
		bool operator!=(const iterator& other) const;
		iterator& operator++();
		iterator operator++(int);
		iterator& operator--();
		iterator operator--(int);
		T& operator*();
	};

	class const_iterator {
	private:
		node* current;
		const_iterator(node* current);
		friend class dlist;

	public:
		const_iterator();
		const_iterator(const const_iterator& other);
		const_iterator& operator=(const const_iterator& other);
		bool operator==(const const_iterator& other) const;
		bool operator!=(const const_iterator& other) const;
		const_iterator& operator++();
		const_iterator operator++(int);
		const_iterator& operator--();
		const_iterator operator--(int);
		const T& operator*() const;
	};

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;

	iterator insert(iterator it, T e);
	iterator remove(iterator it);
};

#include <klibc/STL/dlist_/dlist_imp.hpp>

#endif
