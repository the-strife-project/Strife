#ifndef STDIO_H
#define STDIO_H

#include <kernel/klibc/STL/string>

// Input
string getch();
string readLine();

// Output
void writec(char c);
void printf(const char* format, ...);

#endif
