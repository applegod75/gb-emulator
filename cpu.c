#include "cpu.h"
#include "instruction_map.h"
#include "memory_map.h"

uint8_t isBigEndian(){
    int i = 1;
    return ! *((char*)&i);
};

CpuState* cpu_init(){
    CpuState* rtn = (CpuState*)malloc(sizeof(CpuState));
    void* registers = malloc(6 * sizeof(uint16_t));
    rtn->AF = (uint16_t*)registers;
    // prevent automatic indexing from making my code less readable
    rtn->BC = (uint16_t*)((uint8_t*)registers + 2);
    rtn->DE = (uint16_t*)((uint8_t*)registers + 4);
    rtn->HL = (uint16_t*)((uint8_t*)registers + 6);
    rtn->SP = (uint16_t*)((uint8_t*)registers + 8);
    rtn->PC = (uint16_t*)((uint8_t*)registers + 10);
    
    if (isBigEndian()){
        rtn->A =  (uint8_t*)rtn->AF;
        rtn->B =  (uint8_t*)rtn->BC;
        rtn->C = ((uint8_t*)rtn->BC + 1);
        rtn->D =  (uint8_t*)rtn->DE;
        rtn->E = ((uint8_t*)rtn->DE + 1);
        rtn->H =  (uint8_t*)rtn->HL;
        rtn->L = ((uint8_t*)rtn->HL + 1);
    } else {
        rtn->A = ((uint8_t*)rtn->AF + 1);
        rtn->C =  (uint8_t*)rtn->BC;
        rtn->B = ((uint8_t*)rtn->BC + 1);
        rtn->E =  (uint8_t*)rtn->DE;
        rtn->D = ((uint8_t*)rtn->DE + 1);
        rtn->L =  (uint8_t*)rtn->HL;
        rtn->H = ((uint8_t*)rtn->HL + 1);
    }

    return rtn;
}

void cpu_cycle(State* state){
    instruction_map[mm_read(state, *state->cpu->PC)](state);
}

void cpu_free(CpuState* state){
    // registers are allocated as contiguous memory, so I can only free
    // the first register, which will free all of them.
    free(state->AF);
}
