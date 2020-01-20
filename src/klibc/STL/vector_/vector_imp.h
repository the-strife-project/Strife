#ifndef VECTOR_IMP
#define VECTOR_IMP

#define VECTOR_INITIAL_SIZE 1

template<typename T> vector<T>::vector() {
	data = new T[VECTOR_INITIAL_SIZE];
	sz = 0;
	allocated = VECTOR_INITIAL_SIZE;
}

template<typename T> vector<T>::vector(const vector<T>& other) {
	sz = other.sz;
	allocated = other.allocated;

	data = new T[allocated];
	for(size_t i=0; i<sz; i++) data[i] = other.data[i];
}

template<typename T> vector<T>::~vector() {
	delete [] data;
	data = 0;
}

template<typename T> size_t vector<T>::size() const { return sz; }


// Private methods.
template<typename T> void vector<T>::more() {
	allocated *= 2;

	T* newdata = new T[allocated];
	for(size_t i=0; i<sz; i++) newdata[i] = data[i];

	delete [] data;
	data = newdata;
}

template<typename T> void vector<T>::disp_right(size_t idx, size_t count) {
	if(sz + count > allocated) {
		// We need to reallocate.
		allocated *= 2;

		T* newdata = new T[allocated];
		for(size_t i=0; i<idx; i++) newdata[i] = data[i];
		for(size_t i=idx+count; i<((sz-idx) + count); i++) newdata[i] = data[i];

		delete [] data;
		data = newdata;
	} else {
		// No need to reallocate.
		for(size_t i=sz+count-1; i>=idx+count; i--) data[i] = data[i-count];
	}

	sz += count;
}

template<typename T> void vector<T>::disp_left(size_t idx, size_t count) {
	for(size_t i=idx-count; i<sz-count; i++) data[i] = data[i+count];
	sz -= count;
}


// Back.
template<typename T> void vector<T>::push_back(T e) {
	data[sz++] = e;
	if(sz == allocated) more();
}
template<typename T> void vector<T>::pop_back() { sz--; }
template<typename T> T& vector<T>::back() { return data[sz-1]; }
template<typename T> const T& vector<T>::back() const { return data[sz-1]; }


// Front.
template<typename T> void vector<T>::push_front(T e) {
	disp_right(0, 1);
	data[0] = e;
}
template<typename T> void vector<T>::pop_front() { disp_left(1, 1); }
template<typename T> T& vector<T>::front() { return data[0]; }
template<typename T> const T& vector<T>::front() const { return data[0]; }


// Operators.
template<typename T> vector<T>& vector<T>::operator=(const vector<T>& other) {
	sz = other.sz;
	allocated = other.allocated;

	data = new T[allocated];
	for(size_t i=0; i<sz; i++) data[i] = other.data[i];
}
template<typename T> T& vector<T>::operator[](size_t idx) { return data[idx]; }
template<typename T> const T& vector<T>::operator[](size_t idx) const { return data[idx]; }
template<typename T> bool vector<T>::operator==(const vector<T>& other) const {
	if(sz != other.sz) return false;
	for(size_t i=0; i<sz; i++)
		if(data[i] != other.data[i]) return false;

	return true;
}
template<typename T> bool vector<T>::operator!=(const vector<T>& other) const  { return !(*this == other); }


template<typename T> typename vector<T>::iterator vector<T>::begin() { return data; }
template<typename T> typename vector<T>::iterator vector<T>::end() { return data+sz; }
template<typename T> typename vector<T>::const_iterator vector<T>::cbegin() const { return data; }
template<typename T> typename vector<T>::const_iterator vector<T>::cend() const { return data+sz; }

#endif
