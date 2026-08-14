#include "chip8.h"

void chip8Initialize(struct chip8* c){
	//first 512 are reserved for interpreter (outdated but i'm keeping it for authenticity)
	c->PC = 0x200;
	c->I = 0;	
	//set the memory to 0 again
	for (int i=0 ; i<4096 ; i++){
		c->memory[i] = 0;
	}

	//set the stack to 0? i think? registers happens to be also 16 so im using the same loop
	for (int i=0 ; i<16 ; i++){
		c->stack[i] = 0;
		c->V[i] = 0;
	}

	//set the timers to 0
	c->delay_timer = 0;
	c->sound_timer = 0;
	c->sp->0
}

void chip8EmulateCycle(struct chip8* c){
	//fetch code
	unsigned short PC = c->PC;
	//opcodes are 2 bytes, we need to fetch 2 elements thus opc1 is the most significant part
	unsigned char opc1 = c->memory[PC];
	c->PC++;
	unsigned char opc2 = c->memory[PC];
	c->PC++;
	//now merge them into 1 opcode, using bitwise or and shifting opc1 a byte to the left
	opcode = opc1 << 8 | opc2;
	//decode opcode
	//execute opcode
	//update timers
}

