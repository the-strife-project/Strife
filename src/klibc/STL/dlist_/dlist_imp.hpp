#ifndef DLIST_IMP
#define DLIST_IMP

template<typename T> dlist<T>::dlist() {
	first = last = 0;
	sz = 0;
}

template<typename T> dlist<T>::dlist(const dlist<T>& other) { *this = other; }
template<typename T> dlist<T>::dlist(dlist<T>&& other) : first(other.first), last(other.last), sz(other.sz) {
	other.first = other.last = nullptr;
	other.sz = 0;
}
template<typename T> dlist<T>::~dlist() { clear(); }
template<typename T> size_t dlist<T>::size() const { return sz; }
template<typename T> void dlist<T>::clear() {
	node* current = first;
	while(current) {
		node* next = current->next;
		delete current;
		current = next;
	}

	first = last = 0;
}


// Back.
template<typename T> void dlist<T>::push_back(T e) {
	node* newnode = new node;
	newnode->data = e;
	newnode->next = 0;
	newnode->prev = last;

	if(last) last->next = newnode;
	last = newnode;
	if(!sz) first = last;
	++sz;
}
template<typename T> void dlist<T>::pop_back() {
	node* aux = last;
	last = last->prev;
	last->next = 0;
	delete aux;
	--sz;
	if(!sz) first = last;
}
template<typename T> T& dlist<T>::back() { return last->data; }
template<typename T> const T& dlist<T>::back() const { return last->data; }


// Front.
template<typename T> void dlist<T>::push_front(T e) {
	node* newnode = new node;
	newnode->data = e;
	newnode->next = first;
	newnode->prev = 0;

	first = newnode;
	if(!sz) last = first;
	sz++;
}
template<typename T> void dlist<T>::pop_front() {
	node* aux = first;
	first = first->next;
	delete aux;
	sz--;
	if(!sz) last = first;
}
template<typename T> T& dlist<T>::front() { return first->data; }
template<typename T> const T& dlist<T>::front() const { return first->data; }


// Operators.
template<typename T> dlist<T>& dlist<T>::operator=(const dlist<T>& other) {
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
template<typename T> dlist<T>& dlist<T>::operator=(dlist<T>&& other) {
	first = other.first;
	last = other.last;
	sz = other.sz;
	other.first = other.last = nullptr;
	other.sz = 0;
}
template<typename T> bool dlist<T>::operator==(const dlist<T>& other) const {
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
template <typename T> bool dlist<T>::operator!=(const dlist<T>& other) const { return !(*this == other); }


// iterator
template<typename T> dlist<T>::iterator::iterator(node* current) : current(current) {}
template<typename T> dlist<T>::iterator::iterator() {}
template<typename T> dlist<T>::iterator::iterator(const dlist<T>::iterator& other) : current(other.current) {}
template<typename T> typename dlist<T>::iterator& dlist<T>::iterator::operator=(const dlist<T>::iterator& other) {
	current = other.current;
	return *this;
}
template<typename T> bool dlist<T>::iterator::operator==(const dlist<T>::iterator& other) const { return (current == other.current); }
template<typename T> bool dlist<T>::iterator::operator!=(const dlist<T>::iterator& other) const { return (current != other.current); }
template<typename T> typename dlist<T>::iterator& dlist<T>::iterator::operator++() {
	current = current->next;
	return *this;
}
template<typename T> typename dlist<T>::iterator dlist<T>::iterator::operator++(int) {
	iterator ret = *this;
	current = current->next;
	return ret;
}
template<typename T> typename dlist<T>::iterator& dlist<T>::iterator::operator--() {
	current = current->prev;
	return *this;
}
template<typename T> typename dlist<T>::iterator dlist<T>::iterator::operator--(int) {
	iterator ret = *this;
	current = current->prev;
	return ret;
}
template<typename T> T& dlist<T>::iterator::operator*() { return current->data; }


// const_iterator
template<typename T> dlist<T>::const_iterator::const_iterator(node* current) : current(current) {}
template<typename T> dlist<T>::const_iterator::const_iterator() {}
template<typename T> dlist<T>::const_iterator::const_iterator(const const_iterator& other) : current(other.current) {}
template<typename T> typename dlist<T>::const_iterator& dlist<T>::const_iterator::operator=(const const_iterator& other) { current = other.current; }
template<typename T> bool dlist<T>::const_iterator::operator==(const const_iterator& other) const { return (current == other.current); }
template<typename T> bool dlist<T>::const_iterator::operator!=(const const_iterator& other) const { return (current != other.current); }
template<typename T> typename dlist<T>::const_iterator& dlist<T>::const_iterator::operator++() {
	current = current->next;
	return *this;
}
template<typename T> typename dlist<T>::const_iterator dlist<T>::const_iterator::operator++(int) {
	const_iterator ret = *this;
	current = current->next;
	return ret;
}
template<typename T> typename dlist<T>::const_iterator& dlist<T>::const_iterator::operator--() {
	if(current)
		current = current->prev;
	else
		current = last;
	return *this;
}
template<typename T> typename dlist<T>::const_iterator dlist<T>::const_iterator::operator--(int) {
	const_iterator ret = *this;
	if(current)
		current = current->prev;
	else
		current = last;
	return ret;
}
template<typename T> const T& dlist<T>::const_iterator::operator*() const { return current->data; }


// begin and end
template<typename T> typename dlist<T>::iterator dlist<T>::begin() { return iterator(first); }
template<typename T> typename dlist<T>::iterator dlist<T>::end() { return iterator(0); }
template<typename T> typename dlist<T>::const_iterator dlist<T>::begin() const { return const_iterator(first); }
template<typename T> typename dlist<T>::const_iterator dlist<T>::end() const { return const_iterator(0); }
template<typename T> typename dlist<T>::const_iterator dlist<T>::cbegin() const { return const_iterator(first); }
template<typename T> typename dlist<T>::const_iterator dlist<T>::cend() const { return const_iterator(0); }


template<typename T> typename dlist<T>::iterator dlist<T>::insert(dlist<T>::iterator it, T e) {
	// In case there are no elements (begin() == end()), end takes preference.
	if(it == end()) {
		push_back(e);
		return iterator(last);
	} else if(it == begin()) {
		push_front(e);
		return begin();
	} else {
		node* newnode = new node;
		newnode->data = e;
		newnode->next = it.current;
		newnode->prev = it.current->prev;
		it.current->prev->next = newnode;
		it.current->prev = newnode;
		++sz;
		return iterator(newnode);
	}
}
template<typename T> typename dlist<T>::iterator dlist<T>::remove(dlist<T>::iterator it) {
	if(it == begin()) {
		pop_front();
		return begin();
	} else if(it == end()) {
		pop_back();
		return end();
	} else {
		node* ret = it.current->next;
		it.current->prev->next = it.current->next;
		it.current->next->prev = it.current->prev;
		delete it.current;
		return iterator(ret);
	}
}

#endif
