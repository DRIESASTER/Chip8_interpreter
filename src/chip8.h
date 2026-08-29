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
	//16 level stack only used for return addresses, each addres is 2 bytes
	unsigned short stack[16];
	unsigned char sp;
	//2 timers, delay and sound
	unsigned char delay_timer;
	unsigned char sound_timer;
  //display
  unsigned long long display[32];
  bool keys[16];
  //waits for key release
	bool awaiting_bounce_delay;
	bool allow_draw;
};


void chip8Initialize(struct chip8* c);

void chip8ClearDisplay(struct chip8 *c);

int chip8LoadGame(struct chip8* c, const char* game);

void updateKeyPress(struct chip8* c, char input, bool press);

void chip8Draw(struct chip8 *c, unsigned char x, unsigned char y, unsigned char n);

void chip8EmulateCycle(struct chip8* c); 

#endif
