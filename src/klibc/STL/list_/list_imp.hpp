#ifndef LIST_IMP
#define LIST_IMP

#include <stdarg.h>

template<typename T> list<T>::list() {
	first = last = 0;
	sz = 0;
}
template<typename T> list<T>::list(const list<T>& other) { *this = other; }
template<typename T> list<T>::list(list<T>&& other) : first(other.first), last(other.last), sz(other.sz) {
	other.first = other.last = nullptr;
	other.sz = 0;
}
template<typename T> list<T>::~list() {
	node* current = first;
	while(current) {
		node* next = current->next;
		delete current;
		current = next;
	}

	first = last = 0;
}
template<typename T> size_t list<T>::size() const { return sz; }


// Back.
template<typename T> void list<T>::push_back(T e) {
	node* newnode = new node;
	newnode->data = e;
	newnode->next = 0;

	if(last) last->next = newnode;
	last = newnode;
	if(!sz) first = last;
	sz++;
}
template<typename T> T& list<T>::back() { return last->data; }
template<typename T> const T& list<T>::back() const { return last->data; }


// Front.
template<typename T> void list<T>::push_front(T e) {
	node* newnode = new node;
	newnode->data = e;
	newnode->next = first;

	first = newnode;
	if(!sz) last = first;
	sz++;
}
template<typename T> void list<T>::pop_front() {
	node* aux = first;
	first = first->next;
	delete aux;
	sz--;
	if(!sz) last = first;
}
template<typename T> T& list<T>::front() { return first->data; }
template<typename T> const T& list<T>::front() const { return first->data; }


// Operators.
template<typename T> list<T>& list<T>::operator=(const list<T>& other) {
	node* current_other = other.first;
	node* current_this = 0;
	while(current_other) {
		node* old_this = current_this;
		current_this = new node;
		current_this->data = current_other->data;
		if(old_this)
			old_this->next = current_this;
		else
			first = current_this;
		current_other = current_other->next;
	}
	last = current_this;
	last->next = 0;
	sz = other.sz;
	return *this;
}
template<typename T> list<T>& list<T>::operator=(list<T>&& other) {
	first = other.first;
	last = other.last;
	sz = other.sz;
	other.first = other.last = nullptr;
	other.sz = 0;
}
template<typename T> bool list<T>::operator==(const list<T>& other) const {
	if(sz != other.sz) return false;
	node* current_other = other.first;
	node* current_this = first;
	while(current_this) {
		if(current_this->data != current_other->data) return false;
		current_this = current_this->next;
		current_other = current_other->next;
	}
	return true;
}
template<typename T> bool list<T>::operator!=(const list<T>& other) const { return !(*this == other); }


// iterator
template<typename T> list<T>::iterator::iterator(node* current) : current(current) {}
template<typename T> list<T>::iterator::iterator() {}
template<typename T> list<T>::iterator::iterator(const iterator& other) : current(other.current) {}
template<typename T> typename list<T>::iterator& list<T>::iterator::operator=(const iterator& other) { current = other.current; }
template<typename T> bool list<T>::iterator::operator==(const iterator& other) const { return (current == other.current); }
template<typename T> bool list<T>::iterator::operator!=(const iterator& other) const { return (current != other.current); }
template<typename T> typename list<T>::iterator& list<T>::iterator::operator++() {
	current = current->next;
	return *this;
}
template<typename T> typename list<T>::iterator list<T>::iterator::operator++(int) {
	iterator ret = *this;
	current = current->next;
	return ret;
}
template<typename T> T& list<T>::iterator::operator*() { return current->data; }


// const_iterator
template<typename T> list<T>::const_iterator::const_iterator(node* current) : current(current) {}
template<typename T> list<T>::const_iterator::const_iterator() {}
template<typename T> list<T>::const_iterator::const_iterator(const const_iterator& other) : current(other.current) {}
template<typename T> typename list<T>::const_iterator& list<T>::const_iterator::operator=(const const_iterator& other) { current = other.current; }
template<typename T> bool list<T>::const_iterator::operator==(const const_iterator& other) const { return (current == other.current); }
template<typename T> bool list<T>::const_iterator::operator!=(const const_iterator& other) const { return (current != other.current); }
template<typename T> typename list<T>::const_iterator& list<T>::const_iterator::operator++() {
	current = current->next;
	return *this;
}
template<typename T> typename list<T>::const_iterator list<T>::const_iterator::operator++(int) {
	const_iterator ret = *this;
	current = current->next;
	return ret;
}
template<typename T> const T& list<T>::const_iterator::operator*() const { return current->data; }


// begin and end
template<typename T> typename list<T>::iterator list<T>::begin() { return iterator(first); }
template<typename T> typename list<T>::iterator list<T>::end() { return iterator(0); }
template<typename T> typename list<T>::const_iterator list<T>::begin() const { return const_iterator(first); }
template<typename T> typename list<T>::const_iterator list<T>::end() const { return const_iterator(0); }
template<typename T> typename list<T>::const_iterator list<T>::cbegin() const { return const_iterator(first); }
template<typename T> typename list<T>::const_iterator list<T>::cend() const { return const_iterator(0); }

#endif
