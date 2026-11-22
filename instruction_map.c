#include "instruction_map.h"
#include "memory_map.h"
#include "sleep.h"

void setZ(State* s, uint8_t v){
    if (v){
        *s->cpu->AF |= (1 << 7);
    } else {
        *s->cpu->AF &= (~(1 << 7));
    }
}

void setSubtraction(State* s, uint8_t v){
    if (v){
        *s->cpu->AF |= (1 << 6);
    } else {
        *s->cpu->AF &= (~(1 << 6));
    }
}

void setHalfCarry(State* s, uint8_t v){
    if (v){
        *s->cpu->AF |= (1 << 5);
    } else {
        *s->cpu->AF &= (~(1 << 5));
    }
}

void setCarry(State* s, uint8_t v){
    if (v){
        *s->cpu->AF |= (1 << 4);
    } else {
        *s->cpu->AF &= (~(1 << 4));
    }
}

// ye no I'm not making this little-endian compatible anymore
uint16_t to_u16(uint8_t high, uint8_t low){
    return ((uint16_t)(high) << 8) | low;
}

void nop(State* state){
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void ld_bc_d16(State* state) {
   *state->cpu->BC = to_u16(
        mm_read(state, *state->cpu->PC + 2),
        mm_read(state, *state->cpu->PC + 1)
    );

   *state->cpu->PC += 3;
   sleep_cycles(state, 3);
}

void ldbc_a(State* state) {
    mm_write(state, *state->cpu->BC, *state->cpu->A);
    (*state->cpu->PC)++;

    sleep_cycles(state, 2);
}

void inc_bc(State* state) {
    (*state->cpu->BC)++;
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void inc_u8(State* s, uint8_t* v){
    setSubtraction(s, 0);
    if (*v == 0xFF) {
        setZ(s, 1);
        setHalfCarry(s, 1);
    } else {
        setHalfCarry(s, 0);
        setZ(s, 0);
    }

    (*v)++;
}

void dec_u8(State* s, uint8_t* v){
    setSubtraction(s, 1);
    if (*v == 0x00) {
        setHalfCarry(s, 1);
        setZ(s, 0);
    } else if (*v == 0x01) {
        setHalfCarry(s, 0);
        setZ(s, 1);
    } else {
        setHalfCarry(s, 0);
        setZ(s, 0);
    }

    (*v)++;
}

void inc_b(State* state){
    inc_u8(state, state->cpu->B);
    (*state->cpu->PC)++;
    sleep_cycle(state); 
}

void dec_b(State* state){
    dec_u8(state, state->cpu->B);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void ld_b_d8(State* state){
    *state->cpu->B = mm_read(state, (*state->cpu->PC) + 1);
    (*state->cpu->PC) += 2;
    sleep_cycles(state, 2);
}

void rlca(State* state){
    setSubtraction(state, 0);
    setHalfCarry(state, 0);
    setZ(state, 0);

    uint8_t high = ((*state->cpu->A) & 0x80) >> 7;
    (*state->cpu->A) = (*state->cpu->A) << 1;
    (*state->cpu->A) |= high;

    if (high) {
        setCarry(state, 1);
    } else {
        setCarry(state, 0);
    }

    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void lda16_sp(State* state){
    uint16_t address = to_u16(
        mm_read(state, (*state->cpu->PC) + 1),
        mm_read(state, (*state->cpu->PC) + 2)
    );

    mm_write(state, address, *(state->cpu->SP + 1));
    mm_write(state, address + 1, *(state->cpu->SP));
    (*state->cpu->PC) += 3;
    sleep_cycles(state, 5);
}

void add_u16(State* state, uint16_t* dst, uint16_t* op){
    setSubtraction(state, 0);
    if ((((*dst) & 0x0F) + ((*op) & 0x0F) & 0x10) == 0x10) {
        setHalfCarry(state, 1);
    }

    if ((((*dst) & 0xFF) + ((*op) & 0xFF) & 0x100) == 0x100) {
        setCarry(state, 1);
    }

    *dst += *op;
}

void add_hl_bc(State* state){
    add_u16(state, state->cpu->HL, state->cpu->BC);
    sleep_cycles(state, 2);
}

InstructionFn instruction_map[0x0A] = {
    nop, ld_bc_d16, ldbc_a, inc_bc, inc_b, dec_b, ld_b_d8, rlca, lda16_sp, add_hl_bc
};


InstructionFn prefixed_instruction_map[0x1] = { nop };
