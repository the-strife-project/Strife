#ifndef STRING_H
#define STRING_H

#include <klibc/STL/vector>

class string {
private:
	vector<char> data;
public:
	string();
	string(const string& other);
	string(const char* other);

	size_t size() const;
	size_t length() const;
	char front() const;
	char back() const;

	// Operations on the string.
	void append(const string& other);
	void appendb(const string& other);	// Append at the beginning.
	void disp_right(size_t idx, size_t count);
	void disp_left(size_t idx, size_t count);
	void invert();
	void pop_front();
	void pop_back();
	void clear();


	// Operators.
	string& operator=(const string& other);
	string& operator=(const char* other);
	char& operator[](size_t idx);
	const char& operator[](size_t idx) const;
	bool operator==(const string& other) const;
	bool operator!=(const string& other) const;

	string& operator+=(const string& other);
	string& operator+=(char c);

	// Iterators.
	typedef typename vector<char>::iterator iterator;
	typedef typename vector<char>::const_iterator const_iterator;

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;
};

#endif
