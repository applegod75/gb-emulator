#include <string.h>
#include <stdio.h>
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

    while (1) {
        cpu_cycle(state);
        printState(state);
    }
}
