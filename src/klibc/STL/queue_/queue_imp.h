#ifndef QUEUE_IMP
#define QUEUE_IMP

template<typename T> size_t queue<T>::size() const { return data.size(); }
template<typename T> void queue<T>::push(T e) { data.push_back(e); }
template<typename T> T& queue<T>::front() { return data.front(); }
template<typename T> const T& queue<T>::front() const { return data.front(); }
template<typename T> T& queue<T>::back() { return data.back(); }
template<typename T> const T& queue<T>::back() const { return data.back(); }
template<typename T> void queue<T>::pop() { data.pop_front(); }

template<typename T> bool queue<T>::operator==(const queue<T>& other) const { return data == other.data; }
template<typename T> bool queue<T>::operator!=(const queue<T>& other) const { return data != other.data; }

#endif
