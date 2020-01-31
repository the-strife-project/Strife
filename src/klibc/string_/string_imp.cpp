#ifndef STRING_IMP
#define STRING_IMP

#include <klibc/string>

string::string() {}
string::string(const string& other) { data = other.data; }
string::string(const char* other) { for(size_t i=0; other[i]; i++) data.push_back(other[i]); }
size_t string::size() const { return data.size(); }
size_t string::length() const { return data.size(); }
char string::front() const { return data.front(); }
char string::back() const { return data.back(); }


// Operations on the string.
void string::append(const string& other) { data += other.data; }
void string::appendb(const string& other) { data.push_front(other.data); }
void string::disp_right(size_t idx, size_t count) { data.disp_right(idx, count); }
void string::disp_left(size_t idx, size_t count) { data.disp_left(idx, count); }
void string::invert() { data.invert(); }
void string::pop_front() { data.pop_front(); }
void string::pop_back() { data.pop_back(); }


// Operators.
string& string::operator=(const string& other) {
	data = other.data;
	return *this;
}
string& string::operator=(const char* other) {
	for(size_t i=0; other[i]; i++) data.push_back(other[i]);
	return *this;
}
char& string::operator[](size_t idx) { return data[idx]; }
const char& string::operator[](size_t idx) const { return data[idx]; }
bool string::operator==(const string& other) const { return data == other.data; }
bool string::operator!=(const string& other) const { return data != other.data; }

string& string::operator+=(const string& other) {
	data += other.data;
	return *this;
}
string& string::operator+=(char c) {
	data.push_back(c);
	return *this;
}

string::iterator string::begin() { return data.begin(); }
string::iterator string::end() { return data.end(); }
string::const_iterator string::begin() const { return data.cbegin(); }
string::const_iterator string::end() const { return data.cend(); }
string::const_iterator string::cbegin() const { return data.cbegin(); }
string::const_iterator string::cend() const { return data.cend(); }
#endif
