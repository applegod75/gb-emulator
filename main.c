#include <string.h>
#include <stdio.h>
#include <SDL3/SDL.h>

#include "argparse.h"
#include "gb_emulator.h"
#include "cpu.h"

void printState(State* state) {
    if (state == NULL) return;
    printf("cpu state: \n");
    printf("\tAF = 0x%X\n", *state->cpu->AF);
    printf("\tBC = 0x%X\n", *state->cpu->BC);
    printf("\tDE = 0x%X\n", *state->cpu->DE);
    printf("\tHL = 0x%X\n", *state->cpu->HL);
    printf("\tSP = 0x%X\n", *state->cpu->SP);
    printf("\tPC = 0x%X\n", *state->cpu->PC);
}

int main(int argc, char** argv){
    Args* a = parseArgs(argc, argv);
    if (a == NULL) return 1;
    State* state = initGameboy(a);
    if (state == NULL) return 1;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* win = SDL_CreateWindow(
        "Gameboy emulator", WIDTH, HEIGHT, 0
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(win, NULL);
    
    SDL_Texture* tex = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_XRGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,
        HEIGHT
    );

    uint8_t running = 1;
    SDL_Event event;
    
    state->start_time = SDL_GetTicksNS();

    while (running) {
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_EVENT_QUIT) running = 0; 
        }
//        printf("0x%X | 0x%X\n", *state->cpu->PC, mm_read(state, *state->cpu->PC));
        cpu_cycle(state);
//        printState(state);

    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    
    freeGameboy(state);
}
