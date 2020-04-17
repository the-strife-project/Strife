#ifndef FONT_H
#define FONT_H

#include <common/types.hpp>
#define FONT_LENGTH 4096

void setFont(uint8_t* f);
void loadFontToVGA();
uint8_t* getFont();

#endif
