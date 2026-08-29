#include <SDL2/SDL.h>
#include "chip8.h"

static SDL_Renderer* renderer;
static SDL_Window* win;
int FRAMES_PER_SEC = 60;
uint32_t last_refresh;

int initDisplay(){
    int result = SDL_CreateWindowAndRenderer(640, 320, 0, &win, &renderer);
    if (result != 0){
        printf("error creating window and renderer SDL: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetRenderDrawColor(renderer, 116, 133, 77, 255);
    SDL_RenderPresent(renderer);
    SDL_UpdateWindowSurface(win);  
    last_refresh = 0;
    return 0;
}

int renderDisplay(const uint64_t display[32]){
    SDL_SetRenderDrawColor(renderer, 179, 207, 112, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 116, 133, 77, 255);
    struct SDL_Rect rect;
    rect.w=10;
    rect.h=10;
    //renderdisplay is 32 lines of 64ints each, each bit is a pixel
    for (int row=0; row<32 ; row++){
        for(int col=0 ; col<64 ; col++){
            bool bit = (display[row] >> (63 - col)) & 1;
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

int displayCycle( uint64_t display[32], bool* allow_draw){
    if (last_refresh == 0) last_refresh = SDL_GetTicks();
    uint32_t current_tick = SDL_GetTicks();
    double refresh_rate_ms = 1000.0 / FRAMES_PER_SEC;
    double elapsed_since_last_refresh = (double)(current_tick - last_refresh);
    double remaining_untill_next_refresh_ms = refresh_rate_ms - elapsed_since_last_refresh;
    printf("%f ms remaining", remaining_untill_next_refresh_ms);
    if(remaining_untill_next_refresh_ms <= 0){
        if(renderDisplay(display) != 0){
            printf("error rendering display SDL: %s\n", SDL_GetError());
            return 1;
        }
        last_refresh = SDL_GetTicks();
        *allow_draw = 1;
    }
    return 0;
}

void killDisplay(){
    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(renderer);
    renderer = NULL;
    win = NULL;
}
