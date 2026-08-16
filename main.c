#include "chip8.h" //cpu core implementation

struct chip8 myChip8;

int main(int argc, char **argv){
	//set up render system and register input callbacks
	////setupGraphics();
	//setupInput();

	//init chip8 system and load game into memory
  chip8Initialize(&myChip8);
  if (chip8LoadGame(&myChip8, "Games/pong.ch8") != 0){
    return 1;
  }
  
	//emulation loop
/*	for(;;){
		//emulate one cycle
		myChip8.emulateCycle(&myChip8, &display);

		//if draw flag is set, update the screen
		if(myChip8.drawFlag){
			drawGraphics();
		}

		//store key press state (press and release)
		myChip8.setKeys();
	}
*/
	return 0;
}


