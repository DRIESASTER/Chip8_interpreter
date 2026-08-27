#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include "chip8.h"

//bool load();
int init(const char* game);
void kill();
int cycle(Uint32* display_start);
int renderDisplay();

SDL_Window* win;
SDL_Surface* winSurface;
SDL_Renderer* renderer;
SDL_AudioDeviceID audioDev;
SDL_AudioSpec want, have;
int INSTRUCTIONS_PER_SEC = 800;
int FRAMES_PER_SEC = 60;

struct chip8 c;
int audioCounter = 0;
float last_note = -1;
int main(int argc, char* argv[]){
    if (argc < 2){
        printf("Please provide a game file as argument: ./bin [GAME]\n");
        return 0;
    }

    const char* game = argv[1];

    if (init(game)) return 1;

    SDL_UpdateWindowSurface(win);  

    Uint32 display_start = SDL_GetTicks();
    while(!cycle(&display_start));

    kill();
    return 0;
}

void audioCallback(void *userdata, Uint8 *stream, int len) {
    float *out = (float*) stream;
    int num_samples = len / sizeof(float);
    for (int i = 0 ; i < num_samples ; i++){
        audioCounter++;
        if(audioCounter % 10 == 0){
            last_note = -last_note;
        }
        out[i] = last_note;
    }
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

    int result = SDL_CreateWindowAndRenderer(640, 320, 0, &win, &renderer);
    if (result != 0){
        printf("error creating window and renderer SDL: %s\n", SDL_GetError());
        return 1;
    }

    want.freq = 8000;
    want.format = AUDIO_F32;
    want.channels = 1;
    want.samples = 512;
    want.callback = audioCallback;
    want.userdata = NULL;
    audioDev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    
    SDL_SetRenderDrawColor(renderer, 0, 55, 25, 255);
    // Update window
    SDL_RenderPresent(renderer);

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
    double display_ms = 1000.0 / FRAMES_PER_SEC;
    double elapsed_display = (double)(end_tick - *display_start);
    double remaining_display = display_ms - elapsed_display;
    if(remaining_display <= 0){
        //await release decrement if bigger than 0
        //also decrease timer (60Hz is equal to fps but can separate if preferred)
        if(c.sound_timer > 0) {
            SDL_PauseAudioDevice(audioDev, 0);
            c.sound_timer--;
        } else SDL_PauseAudioDevice(audioDev, 1);
        if(c.delay_timer > 0) c.delay_timer--;
        c.allow_draw = 1;
        if(renderDisplay() != 0){
            printf("error rendering display SDL: %s\n", SDL_GetError());
            return 1;
        }
        *display_start = end_tick;
    }
    if (remaining > 0){
        SDL_Delay((Uint32)remaining);
    }
    return 0;
}

int renderDisplay(){
    SDL_SetRenderDrawColor(renderer, 97, 124, 117, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 55, 25, 255);
    struct SDL_Rect rect;
    rect.w=10;
    rect.h=10;
    //renderdisplay is 32 lines of 64ints each, each bit is a pixel
    for (int row=0; row<32 ; row++){
        for(int col=0 ; col<64 ; col++){
            bool bit = (c.display[row] >> (63 - col)) & 1;
            if (bit == 1){
                rect.x = col*10;
                rect.y = row*10;
                if (SDL_RenderFillRect(renderer, &rect) != 0){
                    return -1;
                }
            }
        }
    }
    SDL_RenderPresent(renderer);
    return 0;
}

void kill(){
    printf("kill command issued");
    SDL_Delay(500);
    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(renderer);
    renderer = NULL;
    win = NULL;
    SDL_Quit();
    return;
}
