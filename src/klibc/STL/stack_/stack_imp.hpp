#ifndef STACK_IMP
#define STACK_IMP

template<typename T> size_t stack<T>::size() const { return data.size(); }
template<typename T> void stack<T>::push(T e) { data.push_front(e); }
template<typename T> T& stack<T>::top() { return data.front(); }
template<typename T> const T& stack<T>::top() const { return data.front(); }
template<typename T> void stack<T>::pop() { data.pop_front(); }

template<typename T> bool stack<T>::operator==(const stack<T>& other) const { return data == other.data; }
template<typename T> bool stack<T>::operator!=(const stack<T>& other) const { return data != other.data; }

#endif
