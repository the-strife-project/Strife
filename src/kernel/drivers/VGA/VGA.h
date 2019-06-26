#ifndef VGA_H
#define VGA_H

#include <common/types.h>

#define VGA_MISC_PORT 0x3c2
#define VGA_CRTC_INDEX_PORT 0x3d4
#define VGA_CRTC_DATA_PORT 0x3d5
#define VGA_SEQUENCER_INDEX_PORT 0x3c4
#define VGA_SEQUENCER_DATA_PORT 0x3c5
#define VGA_GRAPHICS_CONTROLLER_INDEX_PORT 0x3ce
#define VGA_GRAPHICS_CONTROLLER_DATA_PORT 0x3cf
#define VGA_ATTR_CONTROLLER_INDEX_PORT 0x3c0
#define VGA_ATTR_CONTROLLER_READ_PORT 0x3c1
#define VGA_ATTR_CONTROLLER_WRITE_PORT 0x3c0
#define VGA_ATTR_CONTROLLER_RESET_PORT 0x3da

void VGA_writeRegisters(uint8_t* registers);
uint8_t* VGA_getFrameBufferSegment();

void VGA_putPixelRaw(uint32_t x, uint32_t y, uint8_t colorIndex);
uint8_t VGA_setMode(uint32_t width, uint32_t height, uint32_t colordepth);

#define VGA_WIDTH 320
#define VGA_HEIGHT 200
#define VGA_COLORDEPTH 8

#endif
