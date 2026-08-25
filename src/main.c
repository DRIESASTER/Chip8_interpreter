#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include "chip8.h"

//bool load();
bool init();
void kill();
bool loop();

SDL_Window* win;
SDL_Surface* winSurface;
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
        running = loop();
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

    printf("not erroring here at least\n");
    win = SDL_CreateWindow("GAME", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, SDL_WINDOW_SHOWN);
    if  (!win){
        printf("error creating window SDL: %s\n", SDL_GetError());
        return 1;
    }

    winSurface = SDL_GetWindowSurface(win);
	if (!winSurface) {
        printf("error getting windowSurface SDL: %s\n", SDL_GetError());
		return 1;
	}

    return 0;
}

//returns 0 when it needs to stop
bool loop(){
    SDL_Event e;
    SDL_Keycode key;
    while (SDL_PollEvent(&e) != 0){
        switch (e.type){
            case SDL_QUIT:
                return 0;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                key = e.key.keysym.sym;
                updateKeyPress(&c, key, (bool)e.key.state);
                break;
        }
    }
    chip8EmulateCycle(&c);
    return 1;
}

//what do we really need for a loop, every pressed key should get passed to chip8 or even better just set that key to pressed in chip8? 
//that's really the only eventqueue i care about for now i'd say? 
//and keyup the key should get set to 0 again? 
//makes sense i think? i mean this 'loop' is gonna get checked at the start of every cycle so i think it's good design? every emulation cycle ie 16.667 ms it updates

//
// bool load(){
//     SDL_Surface* temp1 = SDL_LoadBMP("sample.bmp");
//
//     if (!temp1){
//         printf("error loading image SDL: %s\n", SDL_GetError());
//         return 1;
//     }
//
//     //this changes the blitmap format to our windowsurface format, visually does nothing but supposedly more efficient so why not
//     image1 = SDL_ConvertSurface(temp1, winSurface->format, 0);
//     return 0;
// }
//
void kill(){
 //   SDL_FreeSurface(image1);
    SDL_DestroyWindow(win);
    SDL_Quit();
}
