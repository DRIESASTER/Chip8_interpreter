#include "chip8.h"
#include <stdio.h>

#define ROMFILE "testfile.ch8"

static int pass_count = 0;
static int fail_count = 0;

#define CHECK(desc, actual, expected)                                        \
  do {                                                                       \
    if ((actual) == (expected)) {                                           \
      printf("[PASS] %s\n", desc);                                          \
      pass_count++;                                                          \
    } else {                                                                 \
      printf("[FAIL] %s (got %ld, expected %ld)\n", desc, (long)(actual),    \
             (long)(expected));                                              \
      fail_count++;                                                          \
    }                                                                        \
  } while (0)

// writes a single 2-byte opcode into testfile.ch8, overwriting it each time
void write_rom(unsigned char b1, unsigned char b2) {
  FILE *f = fopen(ROMFILE, "wb");
  fwrite(&b1, 1, 1, f);
  fwrite(&b2, 1, 1, f);
  fclose(f);
}

// convenience: write opcode, load it into chip8 memory, run one cycle
void run_op(struct chip8 *c, unsigned char b1, unsigned char b2) {
  write_rom(b1, b2);
  chip8LoadGame(c, ROMFILE);
  chip8EmulateCycle(c);
}

int main(void) {
  struct chip8 c;

  // 00E0 - CLS: clear the display
  chip8Initialize(&c);
  c.display[0] = 0xFFFFFFFFFFFFFFFFULL;
  run_op(&c, 0x00, 0xE0);
  CHECK("00E0 clears display[0]", c.display[0], 0);

  // 00EE - RET: return from subroutine
  chip8Initialize(&c);
  c.sp = 1;
  c.stack[0] = 0x300;
  run_op(&c, 0x00, 0xEE);
  CHECK("00EE sets PC from stack", c.PC, 0x300);
  CHECK("00EE decrements sp", c.sp, 0);

  // 1NNN - JP addr
  chip8Initialize(&c);
  run_op(&c, 0x12, 0x34);
  CHECK("1NNN jumps to NNN", c.PC, 0x234);

  // 2NNN - CALL addr
  chip8Initialize(&c);
  run_op(&c, 0x23, 0x00);
  CHECK("2NNN jumps to NNN", c.PC, 0x300);
  CHECK("2NNN pushes return addr", c.stack[0], 0x202);
  CHECK("2NNN increments sp", c.sp, 1);

  // 3XNN - SE Vx, NN (skip if equal)
  chip8Initialize(&c);
  c.V[0] = 0x42;
  run_op(&c, 0x30, 0x42);
  CHECK("3XNN skips when V[x] == NN", c.PC, 0x204);

  // 4XNN - SNE Vx, NN (skip if not equal)
  chip8Initialize(&c);
  c.V[0] = 0x01;
  run_op(&c, 0x40, 0x42);
  CHECK("4XNN skips when V[x] != NN", c.PC, 0x204);

  // 5XY0 - SE Vx, Vy
  chip8Initialize(&c);
  c.V[0] = 5;
  c.V[1] = 5;
  run_op(&c, 0x50, 0x10);
  CHECK("5XY0 skips when V[x] == V[y]", c.PC, 0x204);

  // 6XNN - LD Vx, NN
  chip8Initialize(&c);
  run_op(&c, 0x60, 0x42);
  CHECK("6XNN sets V[x] = NN", c.V[0], 0x42);

  // 7XNN - ADD Vx, NN
  chip8Initialize(&c);
  c.V[0] = 0x10;
  run_op(&c, 0x70, 0x05);
  CHECK("7XNN adds NN to V[x]", c.V[0], 0x15);

  // 8XY0 - LD Vx, Vy
  chip8Initialize(&c);
  c.V[0] = 0x11;
  c.V[1] = 0x99;
  run_op(&c, 0x80, 0x10);
  CHECK("8XY0 sets V[x] = V[y]", c.V[0], 0x99);

  // 8XY1 - OR Vx, Vy
  chip8Initialize(&c);
  c.V[0] = 0x0F;
  c.V[1] = 0xF0;
  run_op(&c, 0x80, 0x11);
  CHECK("8XY1 sets V[x] |= V[y]", c.V[0], 0xFF);

  // 8XY2 - AND Vx, Vy
  chip8Initialize(&c);
  c.V[0] = 0x0F;
  c.V[1] = 0xF3;
  run_op(&c, 0x80, 0x12);
  CHECK("8XY2 sets V[x] &= V[y]", c.V[0], 0x03);

  // 8XY3 - XOR Vx, Vy
  chip8Initialize(&c);
  c.V[0] = 0x0F;
  c.V[1] = 0xF3;
  run_op(&c, 0x80, 0x13);
  CHECK("8XY3 sets V[x] ^= V[y]", c.V[0], 0xFC);

  // 8XY4 - ADD Vx, Vy
  chip8Initialize(&c);
  c.V[0] = 10;
  c.V[1] = 20;
  run_op(&c, 0x80, 0x14);
  CHECK("8XY4 adds V[y] to V[x]", c.V[0], 30);

  // 8XY5 - SUB Vx, Vy
  chip8Initialize(&c);
  c.V[0] = 20;
  c.V[1] = 5;
  run_op(&c, 0x80, 0x15);
  CHECK("8XY5 subtracts V[y] from V[x]", c.V[0], 15);

  // 8XY6 - SHR Vx
  chip8Initialize(&c);
  c.V[0] = 0b0110;
  run_op(&c, 0x80, 0x16);
  CHECK("8XY6 shifts V[x] right by 1", c.V[0], 0b0011);

  // 8XY7 - SUBN Vx, Vy
  chip8Initialize(&c);
  c.V[0] = 5;
  c.V[1] = 20;
  run_op(&c, 0x80, 0x17);
  CHECK("8XY7 sets V[x] = V[y] - V[x]", c.V[0], 15);

  // 8XYE - SHL Vx
  chip8Initialize(&c);
  c.V[0] = 0b0110;
  run_op(&c, 0x80, 0x1E);
  CHECK("8XYE shifts V[x] left by 1", c.V[0], 0b1100);

  // 9XY0 - SNE Vx, Vy
  chip8Initialize(&c);
  c.V[0] = 1;
  c.V[1] = 2;
  run_op(&c, 0x90, 0x10);
  CHECK("9XY0 skips when V[x] != V[y]", c.PC, 0x204);

  // ANNN - LD I, addr
  chip8Initialize(&c);
  run_op(&c, 0xA1, 0x23);
  CHECK("ANNN sets I = NNN", c.I, 0x123);

  // BNNN - JP V0, addr
  chip8Initialize(&c);
  c.V[0] = 0x10;
  run_op(&c, 0xB2, 0x00);
  CHECK("BNNN jumps to NNN + V0", c.PC, 0x210);

  // CXNN - RND Vx, NN (result is random, only check it respects the mask)
  chip8Initialize(&c);
  run_op(&c, 0xC0, 0x0F);
  CHECK("CXNN masks random value with NN", c.V[0] & ~0x0F, 0);

  // DXYN - DRW Vx, Vy, N (just confirm it runs without crashing / touches
  // display; pixel-level correctness is not checked here)
  chip8Initialize(&c);
  c.I = 0; // fontset digit 0 lives at address 0
  run_op(&c, 0xD0, 0x05);
  printf("[INFO] DXYN executed (see sprite printout above), display[0] = %llx\n",
         c.display[0]);

  // EX9E / EXA1 - key skip instructions: not implemented in chip8.c yet, so
  // just confirm they don't crash the interpreter.
  chip8Initialize(&c);
  run_op(&c, 0xE0, 0x9E);
  printf("[INFO] EX9E ran (not yet implemented in chip8.c)\n");

  chip8Initialize(&c);
  run_op(&c, 0xE0, 0xA1);
  printf("[INFO] EXA1 ran (not yet implemented in chip8.c)\n");

  // FX07 - LD Vx, DT: not yet implemented
  chip8Initialize(&c);
  c.delay_timer = 0x42;
  run_op(&c, 0xF0, 0x07);
  printf("[INFO] FX07 ran (not yet implemented in chip8.c)\n");

  // FX0A - LD Vx, K: not yet implemented
  chip8Initialize(&c);
  run_op(&c, 0xF0, 0x0A);
  printf("[INFO] FX0A ran (not yet implemented in chip8.c)\n");

  // FX15 - LD DT, Vx
  chip8Initialize(&c);
  c.V[0] = 0x42;
  run_op(&c, 0xF0, 0x15);
  CHECK("FX15 sets delay_timer = V[x]", c.delay_timer, 0x42);

  // FX18 - LD ST, Vx: not yet implemented (no case for it)
  chip8Initialize(&c);
  c.V[0] = 0x42;
  run_op(&c, 0xF0, 0x18);
  printf("[INFO] FX18 ran (not yet implemented in chip8.c)\n");

  // FX1E - ADD I, Vx
  chip8Initialize(&c);
  c.I = 0x100;
  c.V[0] = 0x10;
  run_op(&c, 0xF0, 0x1E);
  CHECK("FX1E adds V[x] to I", c.I, 0x110);

  // FX29 - LD F, Vx: not yet implemented correctly (sets I to fontset byte
  // value rather than the sprite's address), so this is expected to FAIL.
  chip8Initialize(&c);
  c.V[0] = 1; // digit '1', sprite starts at address 1*5 = 5
  run_op(&c, 0xF0, 0x29);
  CHECK("FX29 sets I to sprite address for digit V[x]", c.I, 5);

  // FX33 - LD B, Vx (BCD): not yet implemented
  chip8Initialize(&c);
  c.V[0] = 123;
  run_op(&c, 0xF0, 0x33);
  printf("[INFO] FX33 ran (not yet implemented in chip8.c)\n");

  // FX55 - LD [I], Vx (register dump): not yet implemented
  chip8Initialize(&c);
  c.I = 0x300;
  c.V[0] = 0x11;
  run_op(&c, 0xF0, 0x55);
  printf("[INFO] FX55 ran (not yet implemented in chip8.c)\n");

  // FX65 - LD Vx, [I] (register load): not yet implemented
  chip8Initialize(&c);
  c.I = 0x300;
  run_op(&c, 0xF0, 0x65);
  printf("[INFO] FX65 ran (not yet implemented in chip8.c)\n");

  printf("\n%d passed, %d failed\n", pass_count, fail_count);
  return fail_count != 0;
}
