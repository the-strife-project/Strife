#ifndef LIST_H
#define LIST_H

#include <common/types.hpp>

template<typename T> class list {
private:
	struct node {
		T data;
		node* next;
	};

	node* first;
	node* last;
	size_t sz;

public:
	list();
	list(const list& other);
	list(list&& other);
	~list();

	size_t size() const;

	// Back.
	void push_back(T e);
	// No pop_back(). It's not efficient in a singly-linked list. Use dlist if you need it.
	T& back();
	const T& back() const;

	// Front.
	void push_front(T e);
	void pop_front();
	T& front();
	const T& front() const;

	// Operators.
	list<T>& operator=(const list<T>& other);
	list<T>& operator=(list<T>&& other);
	bool operator==(const list<T>& other) const;
	bool operator!=(const list<T>& other) const;

	// Iterators.
	class iterator {
	private:
		node* current;
		iterator(node* current);
		friend class list;

	public:
		iterator();
		iterator(const iterator& other);
		iterator& operator=(const iterator& other);
		bool operator==(const iterator& other) const;
		bool operator!=(const iterator& other) const;
		iterator& operator++();
		iterator operator++(int);
		T& operator*();
	};

	class const_iterator {
	private:
		node* current;
		const_iterator(node* current);
		friend class list;

	public:
		const_iterator();
		const_iterator(const const_iterator& other);
		const_iterator& operator=(const const_iterator& other);
		bool operator==(const const_iterator& other) const;
		bool operator!=(const const_iterator& other) const;
		const_iterator& operator++();
		const_iterator operator++(int);
		const T& operator*() const;
	};

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;
};

#include <klibc/STL/list_/list_imp.hpp>

#endif
