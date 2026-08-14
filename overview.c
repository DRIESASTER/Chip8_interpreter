//opcode is 2 bytes long
unsigned short opcode;

//memory is 4kB -> to be bit adressable we need log2(4096*8)=15 bits -> 2 bytes
unsigned char memory[4096];

//there are 16 registers of a byte each, the last one is reserved for the 'carry flag'
unsigned char V[16];

//there's an index register and program counter (pc) that can have value 0x000 to 0xFFF (12 bits) -> 2 bytes necessary in c

unsigned short I;
unsigned short PC;

//0x000 - 0x1FF -> chip8 interpreter (first 512 bytes)
//0x050-0xA0 -> used for the built in 4x5 pixel font set (0-F)
//0x200-0xFFF -> program ROM and work RAM

//there is one instruction that draws a sprite to the screen
//drawing is done via XOR and if a pixel is off as result of the drawing the VF register is set (carry flag) this is done for collision detection

//i will implement just 1 byte for each pixel (1 -> on, 0 -> off)
unsigned char display[64*32]

//there's 2 twimers, a byte each, countdown at 60Hz untill 0
unsigned char delay_timer;
unsigned char sound_timer;

//need a stack with 16 levels, each entry is 16 bits as we dicussed earlier we need 15 bits for bit-adressable memory

//we also need a stack pointer to remember where we are. log2(16)=4 -> 1 byte
unsigned short stack[16];
unsigned char sp;

//keypad for input is 16 keys of 1 byte (0-F)
unsigned char key[16];


