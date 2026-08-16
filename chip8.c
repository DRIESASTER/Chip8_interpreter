#include "chip8.h"
#include <stdio.h>


unsigned char chip8_fontset[80] =
{ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


void chip8Initialize(struct chip8* c){
	//first 512 are reserved for interpreter (outdated but i'm keeping it for authenticity)
	c->PC = 0x200;
	c->I = 0;	

  //load the fontset into memory
  for (int i=0 ; i<80 ; i++){
    c->memory[i] = chip8_fontset[i];
  }
	//set the memory to 0 again
	for (int i=80; i<4096 ; i++){
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
	c->sp = 0;

  //clear the display
  chip8ClearDisplay(c);

  printf("initialized\n");
}


void chip8ClearDisplay(struct chip8 *c){
  for (int i=0 ; i<32 ; i++){
    c->display[i] = 0;
  }
}


//loads the game into the adress starting at byte 512
int chip8Loadgame(struct chip8* c, const char* game){
  FILE *pGame = fopen(game, "rb");
  if (pGame == NULL){
    printf("game file not found\n");
    return 1;
  }
  int ch;
  unsigned int addres = 0x200;
  while((ch = fgetc(pGame)) != EOF){
     if (addres > 0xE9F){
      printf("ERROR gamefile is too large to fit into memory\n");
      fclose(pGame); 
      return 1;
    }

    c->memory[addres] = ch; addres++;
  }
  fclose(pGame);
  return 0;
}

void chip8EmulateCycle(struct chip8* c){
	//fetch code
	unsigned short PC = c->PC;
	
	//opcodes are 2 bytes, we need to fetch 2 elements thus opc1 is the most significant part
	unsigned short opcode = c->memory[PC] << 8 | c->memory[PC+1];
/*	
	//decode opcode
  //reads the first 4 bits
  switch (opcode & 0x0FFF){
    case 0x000{
      //3 cases here 
      if (opcode == 0x00E0){
        //clear the display 

      }
    }*/
  //}	
	
	
	//execute opcode
	//update timers
}


//implement all of the opcodes

