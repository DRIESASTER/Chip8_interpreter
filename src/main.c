#include <stdio.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL.h>
#include "chip8.h"
#include "audio.h"
#include "display.h"
#include "timers.h"

int init(const char* game);
void kill();
int cycle(Uint32* display_start);

struct chip8 c;
int INSTRUCTIONS_PER_SEC = 600;
int main(int argc, char* argv[]){
    if (argc < 2){
        printf("Please provide a game file as argument: ./bin [GAME]\n");
        return 0;
    }
    const char* game = argv[1];

    if (init(game)) return 1;

    Uint32 display_start = SDL_GetTicks();
    while(!cycle(&display_start));

    kill();
    return 0;
}


int init(const char* game){
    chip8Initialize(&c);
    if (chip8LoadGame(&c, game) != 0) {
        return 1;
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }

    initAudio();  
    initDisplay();
    return 0;
}

//returns 1 when it needs to stop
int cycle(Uint32* display_start){
    Uint32 start_tick = SDL_GetTicks();
    SDL_Event e;
    SDL_Keycode key;
    while (SDL_PollEvent(&e) != 0){
        switch (e.type){
            case SDL_QUIT:
                return 1;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                key = e.key.keysym.sym;
                updateKeyPress(&c, key, (bool)e.key.state);
                break;
        }
    }

    chip8EmulateCycle(&c);

    Uint32 end_tick = SDL_GetTicks();
    double instruction_ms = 1000.0 / INSTRUCTIONS_PER_SEC;   
    double elapsed = (double)(end_tick - start_tick);
    double remaining = instruction_ms - elapsed;

    if (displayCycle(c.display, &c.allow_draw) != 0) return 1;
    timerCycle(&c.sound_timer, &c.delay_timer);
    if (remaining > 0){
        SDL_Delay((Uint32)remaining);
    }
    return 0;
}


void kill(){
    printf("kill command issued");
    killDisplay();
    killAudio();
    SDL_Quit();
    return;
}
