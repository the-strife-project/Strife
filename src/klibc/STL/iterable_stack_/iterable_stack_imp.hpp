#ifndef ITERABLE_STACK_IMP
#define ITERABLE_STACK_IMP

#include <klibc/STL/iterable_stack>

template<typename T> typename iterable_stack<T>::iterator iterable_stack<T>::begin() { return stack<T>::data.begin(); }
template<typename T> typename iterable_stack<T>::iterator iterable_stack<T>::end() { return stack<T>::data.end(); }
template<typename T> typename iterable_stack<T>::const_iterator iterable_stack<T>::begin() const { return stack<T>::data.begin(); }
template<typename T> typename iterable_stack<T>::const_iterator iterable_stack<T>::end() const { return stack<T>::data.end(); }
template<typename T> typename iterable_stack<T>::const_iterator iterable_stack<T>::cbegin() const { return stack<T>::data.cbegin(); }
template<typename T> typename iterable_stack<T>::const_iterator iterable_stack<T>::cend() const { return stack<T>::data.cend(); }

#endif
