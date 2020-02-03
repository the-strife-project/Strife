#ifndef WINDOW_IMP
#define WINDOW_IMP

template<typename T> size_t window<T>::size() const { return left_st.size() + right_st.size(); }

template<typename T> T& window<T>::get() { return left_st.top(); }
template<typename T> const T& window<T>::get() const { return left_st.top(); }

template<typename T> void window<T>::put(const T& e) { left_st.push(e); }
template<typename T> void window<T>::remove() { left_st.pop(); }
template<typename T> void window<T>::clear() {
	while(left_st.size()) left_st.pop();
	while(right_st.size()) right_st.pop();
}

template<typename T> void window<T>::left() {
	right_st.push(left_st.top());
	left_st.pop();
}
template<typename T> void window<T>::right() {
	left_st.push(right_st.top());
	right_st.pop();
}

template<typename T> size_t window<T>::nleft() const { return left_st.size(); }
template<typename T> size_t window<T>::nright() const { return right_st.size(); }
template<typename T> bool window<T>::isBeginning() const { return left_st.size() == 0; }
template<typename T> bool window<T>::isFirst() const { return left_st.size() == 1; }
template<typename T> bool window<T>::isLast() const { return right_st.size() == 0; }

template<typename T> window<T>& window<T>::operator++() {
	right();
	return *this;
}
template<typename T> window<T> window<T>::operator++(int) {
	window<T> ret = *this;
	right();
	return ret;
}
template<typename T> window<T>& window<T>::operator--() {
	left();
	return *this;
}
template<typename T> window<T> window<T>::operator--(int) {
	window<T> ret = *this;
	left();
	return ret;
}

template<typename T> typename window<T>::iterator window<T>::begin() { return right_st.begin(); }
template<typename T> typename window<T>::iterator window<T>::end() { return right_st.end(); }
template<typename T> typename window<T>::const_iterator window<T>::begin() const { return right_st.begin(); }
template<typename T> typename window<T>::const_iterator window<T>::end() const { return right_st.end(); }

#endif
