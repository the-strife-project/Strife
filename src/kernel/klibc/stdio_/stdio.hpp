#ifndef STDIO_H
#define STDIO_H

#include <string>

// Input
string getch();
string readLine();

// Output
void writec(char c);
void printf(const char* format, ...);

#endif
