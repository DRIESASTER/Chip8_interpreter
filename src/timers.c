#include "timers.h" 
#include "audio.h"
#include <SDL2/SDL_timer.h>

static int TIMERS_HZ = 60;
static uint32_t last_refresh = 0;

void timerCycle(unsigned char* sound_timer, unsigned char* delay_timer){
    uint32_t end_tick = SDL_GetTicks();
    // double instruction_ms = 1000.0 / INSTRUCTIONS_PER_SEC;   
    // double elapsed = (double)(end_tick - start_tick);
    // double remaining = instruction_ms - elapsed;
    double timer_refresh_rate_ms = 1000.0 / TIMERS_HZ;
    double elapsed_since_last_countdown = (double)(end_tick - last_refresh);
    double remaining_untill_next_countdown = timer_refresh_rate_ms - elapsed_since_last_countdown;
    printf("remaining %f: ", remaining_untill_next_countdown);
    if(remaining_untill_next_countdown <= 0){
        //await release decrement if bigger than 0
        //also decrease timer (60Hz is equal to fps but can separate if preferred)
        changeAudioStatus(!(*sound_timer > 0));
        if(*sound_timer > 0) {
            *sound_timer = *sound_timer - 1;
        }
        if(*delay_timer > 0) *delay_timer = *delay_timer - 1;
        last_refresh = end_tick;
    }
    return;
}

