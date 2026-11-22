#include <SDL3/SDL.h>
#include "sleep.h"
#include <stdio.h>

void sleep_cycle(State* state){
    state->current_cycle++;
    if (state->current_cycle % 10000 == 0){ 
        Uint64 target = state->start_time + state->current_cycle * CPU_CLOCK_TIME * 1e9;
        Uint64 now = SDL_GetTicksNS();
        if (now < target && target - now > 1e3) {
            SDL_DelayNS(target - now);
        } else {
            printf("too slow by %lluns\n", now - target);
        }
    }
}

void sleep_cycles(State* state, uint8_t a){
    state->current_cycle += a;
}
