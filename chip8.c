#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

unsigned char chip8_fontset[80] = {
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

void chip8Initialize(struct chip8 *c) {
  // first 512 are reserved for interpreter (outdated but i'm keeping it for
  // authenticity)
  c->PC = 0x200;
  c->I = 0;

  // load the fontset into memory
  for (int i = 0; i < 80; i++) {
    c->memory[i] = chip8_fontset[i];
  }
  // set the memory to 0 again
  for (int i = 80; i < 4096; i++) {
    c->memory[i] = 0;
  }

  // set the stack to 0? i think? registers happens to be also 16 so im using
  // the same loop
  for (int i = 0; i < 16; i++) {
    c->stack[i] = 0;
    c->V[i] = 0;
  }

  // set the timers to 0
  c->delay_timer = 0;
  c->sound_timer = 0;
  c->sp = 0;

  // clear the display
  chip8ClearDisplay(c);
}

void chip8ClearDisplay(struct chip8 *c) {
  for (int i = 0; i < 32; i++) {
    c->display[i] = 0;
  }
}

// loads the game into the adress starting at byte 512
int chip8LoadGame(struct chip8 *c, const char *game) {
  FILE *pGame = fopen(game, "rb");
  if (pGame == NULL) {
    printf("game file not found\n");
    return 1;
  }
  int ch;
  unsigned int addres = 0x200;
  while ((ch = fgetc(pGame)) != EOF) {
    if (addres > 0xE9F) {
      printf("ERROR gamefile is too large to fit into memory\n");
      fclose(pGame);
      return 1;
    }
    c->memory[addres] = ch;
    addres++;
  }
  fclose(pGame);
  return 0;
}

void chip8Draw(struct chip8 *c, unsigned char x, unsigned char y,
               unsigned char n) {
  // Draws a sprite at coordinate x,y, that has a width of 8 pixels (1 byte) and
  // height of n pixels each row of 8 pixels is read as bit-coded starting from
  // memory location c->I I's value does not change after the execution of this
  // instruction c->V[0xF] is set to 1 if any screen pixels are flipped from set
  // to unset (1->0) and to 0 if it doesn't occur
  printf("draw at x=%d y=%d n=%d I=%d\n", x, y, n, c->I);
  unsigned short pixels_loc = c->I;
  for (int i = 0; i < n; i++) {
    // we create a row -> 64 bits -> we need to shift to location x
    unsigned char c_pixels = c->memory[pixels_loc + i];
    // we need to cast to long long because otherwise we only get 8 bits and
    // thus can only use the last 1/8th of the display....
    long long c_row = (long long)c_pixels << (63 - x - 8);
    (c->display)[y + i] ^= c_row;
  }

  // display the display in the terminal
  // printf("\033[H\033[J"); // ANSI escape: clear screen + move cursor home
  for (int row = 0; row < 32; row++) {
    for (int col = 0; col < 64; col++) {
      // bit-packed row: MSB = leftmost pixel
      unsigned long long bit = (c->display[row] >> (63 - col)) & 1;
      putchar(bit ? '#' : ' ');
    }
    putchar('\n');
  }
}

void chip8EmulateCycle(struct chip8 *c) {
  // fetch code
  // opcodes are 2 bytes, we need to fetch 2 elements thus opc1 is the most
  // significant part
  unsigned short opcode = c->memory[c->PC] << 8 | c->memory[c->PC + 1];
  c->PC += 2;

  // decode opcode
  // pull out the common fields once so every case below reads like the opcode
  // table instead of re-deriving (and risking re-breaking) the same masks each
  // time
  unsigned char x = (opcode & 0x0F00) >> 8; // X register index
  unsigned char y = (opcode & 0x00F0) >> 4; // Y register index
  unsigned char n = (opcode & 0x000F);      // low nibble
  unsigned char nn = (opcode & 0x00FF);     // low byte / constant
  unsigned short nnn = (opcode & 0x0FFF);   // low 12 bits / address

  // reads the first 4 bits to pick the instruction group
  switch (opcode & 0xF000) {
  case 0x0000:
    switch (nn) {
    case 0xE0:
      chip8ClearDisplay(c);
      break;

    case 0xEE:
      c->sp--;
      c->PC = c->stack[c->sp];
      break;
    }
    break;

  case 0x1000:
    // jumps to address NNN
    c->PC = nnn;
    break;

  case 0x2000:
    // calls subroutine at NNN
    if (c->sp >= 16) {
      printf("ERROR stack overflow\n");
      break;
    }
    c->stack[c->sp] = c->PC;
    c->sp++;
    c->PC = nnn;
    break;

  case 0x3000:
    // 0x3XNN
    // skips next instruction if V[x] == NN
    if (c->V[x] == nn) {
      c->PC += 2;
    }
    break;

  case 0x4000:
    // 0x4XNN
    // skips next instruction if V[x] != NN
    if (c->V[x] != nn) {
      c->PC += 2;
    }
    break;

  case 0x5000:
    // 0x5XY0
    // if V[x] == V[y] skips next instruction
    if (c->V[x] == c->V[y]) {
      c->PC += 2;
    }
    break;

  case 0x6000:
    c->V[x] = nn;
    break;

  case 0x7000:
    c->V[x] += nn;
    break;

  case 0x8000:
    // 0x8XYZ
    switch (n) {
    case 0x1:
      // set V[x] = V[y]
      c->V[x] = c->V[y];
      break;
    case 0x2:
      c->V[x] |= c->V[y];
      break;
    case 0x3:
      // sets VX to VX xor XY
      c->V[x] ^= c->V[y];
      break;
    case 0x4:
      c->V[x] += c->V[y];
      break;
    case 0x5:
      c->V[x] -= c->V[y];
      break;
    case 0x6:
      c->V[x] >>= 1;
      break;
    case 0x7:
      c->V[x] = c->V[y] - c->V[x];
      break;
    case 0xE:
      c->V[x] <<= 1;
      break;
    default:
      printf("ERROR unknown 8XY%X opcode: %X\n", n, opcode);
      break;
    }
    break;
  case 0x9000:
    if (c->V[x] != c->V[y])
      c->PC++;
    break;
  case 0xA000:
    c->I = nnn;
    break;
  case 0xB000:
    c->PC = c->V[0] + nnn;
    break;
  case 0xC000:
    srand(time(NULL));
    c->V[x] = rand() % 256 & nn;
    break;
  case 0xD000:
    // display
    chip8Draw(c, c->V[x], c->V[y], n);
    break;
  case 0xE000:
    switch (n) {
    case 0xE:
      printf("TODO 0xE9E");
      // if(key() == c->V[x]);
      // c->PC++;
      break;
    case 0x1:
      // TODO
      printf("todo 0xEA1");
      // if (key() != c->V[x]);
      break;
    default:
      printf("illegal operation 0x%x", opcode);
      break;
    }
    break;
  case 0xF000:
    switch (n) {
    case 0x7:
      // delay_timer
      printf("delay_timer_TODO");
      break;
    case 0xA:
      // c->V[x] = get_key();
      printf("get_key_todo");
      break;
    case 0x5:
      switch (nn) {
      case 0x15:
        c->delay_timer = c->V[x];
        break;
      case 0x55:
        // reg_dump(c->V[x], &(c->I));
        printf("todo reg_dump 0x55");
        break;
      case 0x65:
        // reg_load(c->V[x], &(c->I));
        printf("todo reg_load 0x65");
        break;
      default:
        printf("illegal 0xFXX5 operation");
        break;
      }
      break;
    case 0xE:
      c->I += c->V[x];
      break;
    case 0x9:
      // most likely incorrect
      c->I = chip8_fontset[c->V[x]];
      break;
    default:
      printf("invalid 0xF000 operation");
      break;
    }
    break;
  default:
    printf("ERROR opcode: %X not found\n", opcode);
  }
}
