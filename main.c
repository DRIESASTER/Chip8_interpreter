#include
#include //openGL graphics and input
#include "chip8.h" //cpu core implementation

chip8 myChip8;

int main(int argc, char **argv){
	//set up render system and register input callbacks
	setupGraphics();
	setupInput();

	//init chip8 system and load game into memory
	myChip8.initialize();
	myChip8.loadGame("pong");

	//emulation loop
	for(;;){
		//emulate one cycle
		myChip8.emulateCycle();

		//if draw flag is set, update the screen
		if(myChip8.drawFlag){
			drawGraphics();
		}

		//store key press state (press and release)
		myChip8.setKeys();
	}

	return 0;
}
