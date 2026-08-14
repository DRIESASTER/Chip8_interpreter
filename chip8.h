#ifndef CHIP8_H
#define CHIP8_H

struct chip8{
	//4kB mem
	unsigned char memory[4096];
	//16 registers 
	unsigned char V[16];
	//index register and program counter
	unsigned short I;
	unsigned short PC;
	//16 level stack 
	unsigned short stack[16];
	unsigned char sp;
	//2 timers, delay and sound
	unsigned char delay_timer;
	unsigned char sound_timer;
};


void chip8Initialize(struct chip8* c);


void chip8LoadGame(struct chip8* c, const char* game);


void chip8EmulateCycle(struct chip8* c);

#endif
