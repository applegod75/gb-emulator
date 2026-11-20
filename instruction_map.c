#include "instruction_map.h"
#include "memory_map.h"

// ye no I'm not making this little-endian compatible anymore
uint16_t to_u16(uint8_t high, uint8_t low){
    return ((uint16_t)(high) << 8) | low;
}

void nop(State* state){
    (*state->cpu->PC)++;
}

void ld_bc_d16(State* state) {
   *state->cpu->BC = to_u16(
        mm_read(state, *state->cpu->PC + 2),
        mm_read(state, *state->cpu->PC + 1)
    );

   *state->cpu->PC += 3;
}



InstructionFn instruction_map[0x2] = {
    nop, ld_bc_d16
};


InstructionFn prefixed_instruction_map[0x1] = { nop };
