#include "chip8.h" //cpu core implementation
#include <stdio.h>
struct chip8 myChip8;

int main(int argc, char **argv) {
  // set up render system and register input callbacks
  ////setupGraphics();
  // setupInput();

  // init chip8 system and load game into memory
  chip8Initialize(&myChip8);
  printf("wtf\n");
  if (chip8LoadGame(&myChip8, "opcodetest.ch8") != 0) {
    return 1;
  }

  // emulation loop
  for (int i = 0; i < 50; i++) {
    // emulate one cycle
    chip8EmulateCycle(&myChip8);
    printf("PC is at %d", myChip8.PC);
    // if draw flag is set, update the screen
    //		if(myChip8.drawFlag){
    //			drawGraphics();
    //		}

    // store key press state (press and release)
    //		myChip8.setKeys();
  }

  return 0;
}
