#ifndef DISPLAY_H
#define DISPLAY_H
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "chip8.h"

int initDisplay();

int renderDisplay(const uint64_t display[32]);

int displayCycle(uint64_t display[32], bool* allow_draw);

void killDisplay();

#endif
