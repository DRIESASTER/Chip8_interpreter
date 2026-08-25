#include "chip8.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

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

// --- input-handling test helpers ---------------------------------------
// updateKeypad() and awaitKeyAndGet() both read from the real stdin
// stream. To test them without needing an actual human at the keyboard,
// these helpers temporarily swap `stdin` for an in-memory stream
// (fmemopen) containing whatever characters a test wants "typed", then
// restore the real stdin afterward.
static FILE *real_stdin = NULL;

void inject_input(const char *s) {
  real_stdin = stdin;
  stdin = fmemopen((void *)s, strlen(s), "r");
}

void restore_input(void) {
  fclose(stdin);
  stdin = real_stdin;
}

// Runs FX0A (a genuinely blocking opcode) in a forked child with a timeout,
// so that if it hangs, the test suite can report that as an actual FAIL
// instead of hanging itself or silently skipping the check. If the child
// finishes in time, its resulting V[x] is sent back through a pipe so the
// parent can CHECK it normally.
void test_fx0a(void) {
  int pipefd[2];
  pipe(pipefd);

  pid_t pid = fork();
  if (pid == 0) {
    // child: do the actual FX0A test
    close(pipefd[0]);
    alarm(2); // default SIGALRM action is to terminate - our timeout

    struct chip8 c;
    chip8Initialize(&c);
    inject_input("3"); // simulate pressing key '3'
    run_op(&c, 0xF0, 0x0A); // FX0A, X=0
    restore_input();

    int result = c.V[0];
    write(pipefd[1], &result, sizeof(result));
    close(pipefd[1]);
    _exit(0);
  }

  // parent: wait for either a result or the child getting killed by alarm()
  close(pipefd[1]);
  int result;
  ssize_t n = read(pipefd[0], &result, sizeof(result));
  int status;
  waitpid(pid, &status, 0);
  close(pipefd[0]);

  if (n == (ssize_t)sizeof(result)) {
    CHECK("FX0A stores the pressed key's value in V[x]", result, 3);
  } else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGALRM) {
    printf("[FAIL] FX0A hung and had to be killed by a timeout - "
           "updateKeypad() drains stdin before this case runs, so "
           "awaitKeyAndGet() never finds a key to read\n");
    fail_count++;
  } else {
    printf("[FAIL] FX0A child exited without producing a result "
           "(status=%d)\n", status);
    fail_count++;
  }
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

  // --- keypad-driven opcodes: EX9E, EXA1 -----------------------------
  // These go through the real chip8EmulateCycle() -> updateKeypad() path,
  // with stdin swapped for injected input, so the full pipeline (raw
  // characters -> charToKey -> c->keys[] -> opcode dispatch) is what's
  // actually being exercised, not just the dispatch logic in isolation.

  // EX9E - SKP Vx: skip next instruction if key V[x] is pressed.
  // No key typed -> updateKeypad leaves all of c->keys[] false -> no skip.
  chip8Initialize(&c);
  c.V[0] = 5;
  inject_input("");
  run_op(&c, 0xE0, 0x9E);
  restore_input();
  CHECK("EX9E does not skip when V[x]'s key is not pressed", c.PC, 0x202);

  // EX9E - key '5' typed, V[0] == 5 -> should skip.
  chip8Initialize(&c);
  c.V[0] = 5;
  inject_input("5");
  run_op(&c, 0xE0, 0x9E);
  restore_input();
  CHECK("EX9E skips when V[x]'s key is pressed", c.PC, 0x204);

  // EXA1 - SKNP Vx: skip next instruction if key V[x] is NOT pressed.
  // No key typed -> key is not pressed -> should skip.
  chip8Initialize(&c);
  c.V[0] = 5;
  inject_input("");
  run_op(&c, 0xE0, 0xA1);
  restore_input();
  CHECK("EXA1 skips when V[x]'s key is not pressed", c.PC, 0x204);

  // EXA1 - key '5' typed, V[0] == 5 -> key IS pressed -> should NOT skip.
  chip8Initialize(&c);
  c.V[0] = 5;
  inject_input("5");
  run_op(&c, 0xE0, 0xA1);
  restore_input();
  CHECK("EXA1 does not skip when V[x]'s key is pressed", c.PC, 0x202);

  // FX07 - LD Vx, DT: V[x] = delay_timer
  chip8Initialize(&c);
  c.delay_timer = 0x42;
  run_op(&c, 0xF0, 0x07);
  CHECK("FX07 sets V[x] = delay_timer", c.V[0], 0x42);

  // FX0A - LD Vx, K: wait for a keypress, then store it in V[x].
  //
  // This is run inside test_fx0a() (fork + alarm timeout, see above)
  // because it's expected this may genuinely hang: updateKeypad() drains
  // stdin before this case ever runs, so awaitKeyAndGet() typically finds
  // nothing left to read and spins forever. That hang is exactly the bug
  // this test is meant to surface - it reports as a real [FAIL], not a
  // skip.
  test_fx0a();

  // FX15 - LD DT, Vx
  chip8Initialize(&c);
  c.V[0] = 0x42;
  run_op(&c, 0xF0, 0x15);
  CHECK("FX15 sets delay_timer = V[x]", c.delay_timer, 0x42);

  // FX18 - LD ST, Vx: sound_timer = V[x]
  chip8Initialize(&c);
  c.V[0] = 0x42;
  run_op(&c, 0xF0, 0x18);
  CHECK("FX18 sets sound_timer = V[x]", c.sound_timer, 0x42);

  // FX1E - ADD I, Vx
  chip8Initialize(&c);
  c.I = 0x100;
  c.V[0] = 0x10;
  run_op(&c, 0xF0, 0x1E);
  CHECK("FX1E adds V[x] to I", c.I, 0x110);

  // FX29 - LD F, Vx: I should point to the address of the 5-byte sprite
  // for digit V[x], i.e. V[x] * 5
  chip8Initialize(&c);
  c.V[0] = 1; // digit '1', sprite starts at address 1*5 = 5
  run_op(&c, 0xF0, 0x29);
  CHECK("FX29 sets I to sprite address for digit V[x]", c.I, 5);

  // FX33 - LD B, Vx (BCD): store the 3 decimal digits of V[x] at
  // memory[I], memory[I+1], memory[I+2] (hundreds, tens, ones)
  chip8Initialize(&c);
  c.I = 0x300;
  c.V[0] = 123;
  run_op(&c, 0xF0, 0x33);
  CHECK("FX33 stores hundreds digit at memory[I]", c.memory[0x300], 1);
  CHECK("FX33 stores tens digit at memory[I+1]", c.memory[0x301], 2);
  CHECK("FX33 stores ones digit at memory[I+2]", c.memory[0x302], 3);

  // FX55 - LD [I], Vx (register dump): memory[I+i] = V[i] for i = 0..X
  // use X=2 so the loop behavior (not just a single register) gets tested
  chip8Initialize(&c);
  c.I = 0x300;
  c.V[0] = 0x11;
  c.V[1] = 0x22;
  c.V[2] = 0x33;
  run_op(&c, 0xF2, 0x55);
  CHECK("FX55 dumps V[0] to memory[I]", c.memory[0x300], 0x11);
  CHECK("FX55 dumps V[1] to memory[I+1]", c.memory[0x301], 0x22);
  CHECK("FX55 dumps V[2] to memory[I+2]", c.memory[0x302], 0x33);

  // FX65 - LD Vx, [I] (register load): V[i] = memory[I+i] for i = 0..X
  chip8Initialize(&c);
  c.I = 0x300;
  c.memory[0x300] = 0x11;
  c.memory[0x301] = 0x22;
  c.memory[0x302] = 0x33;
  run_op(&c, 0xF2, 0x65);
  CHECK("FX65 loads memory[I] into V[0]", c.V[0], 0x11);
  CHECK("FX65 loads memory[I+1] into V[1]", c.V[1], 0x22);
  CHECK("FX65 loads memory[I+2] into V[2]", c.V[2], 0x33);

  printf("\n%d passed, %d failed\n", pass_count, fail_count);
  return fail_count != 0;
}
