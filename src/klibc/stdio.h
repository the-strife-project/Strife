#ifndef STDIO_H
#define STDIO_H

#include <klibc/string>

// Input
char getch();
string readLine();

// Output
void writec(char c);
void printf(const char* format, ...);

#endif
