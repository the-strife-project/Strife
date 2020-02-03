#ifndef UTILITY_H
#define UTILITY_H

template<typename T> T&& move(T& x) { return static_cast<T&&>(x); }

#endif
