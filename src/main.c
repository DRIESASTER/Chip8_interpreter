#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include "chip8.h"

//bool load();
bool init();
void kill();
bool cycle();
bool renderDisplay();

SDL_Window* win;
SDL_Surface* winSurface;
SDL_Renderer* renderer;
//SDL_Surface* image1;

struct chip8 c;

int main(){
    if (init()) return 1;

    //  if (load()) return 1;
    //
    // SDL_BlitSurface(image1, NULL, winSurface, NULL);
    SDL_UpdateWindowSurface(win); 
    // SDL_Delay(1000);
    bool running = 1;
    while(running){
        running = !cycle();
    }

    kill();
    return 0;
}


bool init(){
    chip8Initialize(&c);
    if (chip8LoadGame(&c, "Games/ibm_logo.ch8") != 0) {
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
	//
	//    winSurface = SDL_GetWindowSurface(win);
	// if (!winSurface) {
	//        printf("error getting windowSurface SDL: %s\n", SDL_GetError());
	// 	return 1;
	// }
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    // Update window
    SDL_RenderPresent(renderer);

    return 0;
}

//returns 1 when it needs to stop
bool cycle(){
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

    if (renderDisplay() != 0){
        printf("error rendering display SDL: %s\n", SDL_GetError());
        return 1;
    }
    Uint32 end_tick = SDL_GetTicks();
    double frame_ms = 1000.0 / 60.0;   
    double elapsed = (double)(end_tick - start_tick);
    double remaining = frame_ms - elapsed;
    if (remaining > 0){
        SDL_Delay((Uint32)remaining);
    }
    else{
        printf("frame too slow, time to render was %f\n", elapsed);
    }
    //im thinking every emulation cycle we also just render the display even if it doesn't change or smt that's not an issue
    return 0;
}

bool renderDisplay(){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
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
    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(renderer);
    renderer = NULL;
    win = NULL;
    SDL_Quit();
    return;
}
