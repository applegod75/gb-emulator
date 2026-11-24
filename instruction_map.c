#include "instruction_map.h"
#include "memory_map.h"
#include "sleep.h"
#include <stdio.h>

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

uint8_t read_d8(State* s){
    return mm_read(s, *s->cpu->PC + 1);
}

void ld_d8_to(State* s, uint8_t* t){
    *t = read_d8(s);
}

uint16_t read_d16(State* s){
    return to_u16(
        mm_read(s, *s->cpu->PC + 2),
        mm_read(s, *s->cpu->PC + 1)
    );
}

void nop(State* state){
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void ld_bc_d16(State* state) {
    *state->cpu->BC = read_d16(state);
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

void rotrc(State* s, uint8_t* v, uint8_t set_zero){
    setSubtraction(s, 0);
    setHalfCarry(s, 0);

    uint8_t low = (*v) & 0x01;
    setCarry(s, low);
    *v = *v >> 1;
    *v |= (low << 7);

    if (set_zero && *v == 0){
        setZ(s, 1);
    } else {
        setZ(s, 0);
    }
}

void rotlc(State* s, uint8_t* v, uint8_t set_zero){
    setSubtraction(s, 0);
    setHalfCarry(s, 0);

    uint8_t high = ((*v) & 0x80) >> 7;
    setCarry(s, high);
    *v = *v << 1;
    *v |= high;

    if (set_zero && *v == 0){
        setZ(s, 1);
    } else {
        setZ(s, 0);
    }
}

void rotr(State* s, uint8_t* v, uint8_t set_zero){
    uint8_t cy = ((*s->cpu->AF) >> 4) & 0x01;
    uint8_t new_cy = (*v & 0x01);
    *v = (*v >> 1) | (cy << 7);
    setCarry(s, new_cy);

    if (set_zero && *v == 0){
        setZ(s, 1);
    } else {
        setZ(s, 0);
    }
}

void rotl(State* s, uint8_t* v, uint8_t set_zero){
    uint8_t cy = ((*s->cpu->AF) >> 4) & 0x01;
    uint8_t new_cy = (*v & 0x80);
    *v = (*v << 1) | cy;
    setCarry(s, new_cy);

    if (set_zero && *v == 0){
        setZ(s, 1);
    } else {
        setZ(s, 0);
    }
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
    rotlc(state, state->cpu->A, 0);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void lda16_sp(State* state){
    uint16_t address = read_d16(state);

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
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void ld_a_bc(State* state){
    *state->cpu->A = mm_read(state, *state->cpu->BC);
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void dec_bc(State* state){
    (*state->cpu->BC)--;
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void inc_c(State* state) {
    inc_u8(state, state->cpu->C);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void dec_c(State* state) {
    dec_u8(state, state->cpu->C);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void ld_c_d8(State* state){
    ld_d8_to(state, state->cpu->C);
    (*state->cpu->PC) += 2;
    sleep_cycles(state, 2);
}

void rrca(State* state){
    rotrc(state, state->cpu->A, 0);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void stop(State* state){
    if (state->debug_mode) {
        printf("CPU:\n");
        printf("\tAF = 0x%X\n", *state->cpu->AF);
        printf("\tBC = 0x%X\n", *state->cpu->BC);
        printf("\tDE = 0x%X\n", *state->cpu->DE);
        printf("\tHL = 0x%X\n", *state->cpu->HL);
        printf("\tPC = 0x%X\n", *state->cpu->PC);
        printf("\tSP = 0x%X\n", *state->cpu->SP);
    } else {
        printf("WARN: reached unimplemented cpu instruction 'STOP'\n");
    }

    (*state->cpu->PC) += 2;
}

void ld_de_d16(State* state){
    *state->cpu->DE = read_d16(state);
    (*state->cpu->PC) += 3;
    sleep_cycles(state, 3);
}

void ldde_a(State* state){
    mm_write(state, *state->cpu->DE, *state->cpu->A);
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void inc_de(State* state){
    (*state->cpu->DE)++;
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void inc_d(State* state){
    inc_u8(state, state->cpu->D);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void dec_d(State* state){
    dec_u8(state, state->cpu->D);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void ld_d_d8(State* state){
    ld_d8_to(state, state->cpu->D);
    (*state->cpu->PC) += 2;
    sleep_cycles(state, 2);
}

void rla(State* state){
    rotl(state, state->cpu->A, 0);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void jr_s8(State* state){
    int8_t d = (int8_t)read_d8(state);
    (*state->cpu->PC) += d;
    sleep_cycles(state, 3);
}

void add_hl_de(State* state){
    add_u16(state, state->cpu->HL, state->cpu->DE);
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void ld_a_de(State* state){
    *state->cpu->A = mm_read(state, *state->cpu->DE);
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void dec_de(State* state){
    (*state->cpu->DE)--;
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void inc_e(State* state){
    inc_u8(state, state->cpu->E);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void dec_e(State* state){
    dec_u8(state, state->cpu->E);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void ld_e_d8(State* state){
    (*state->cpu->E) = read_d8(state);
    (*state->cpu->PC) += 2;
    sleep_cycles(state, 2);
}

void rra(State* state) {
    rotr(state, state->cpu->A, 0);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

InstructionFn instruction_map[0x20] = {
    nop,  ld_bc_d16, ldbc_a, inc_bc, inc_b, dec_b, ld_b_d8, rlca, lda16_sp, add_hl_bc, ld_a_bc, dec_bc, inc_c, dec_c, ld_c_d8, rrca,
    stop, ld_de_d16, ldde_a, inc_de, inc_d, dec_d, ld_d_d8, rla,  jr_s8,    add_hl_de, ld_a_de, dec_de, inc_e, dec_e, ld_e_d8, rra
};


InstructionFn prefixed_instruction_map[0x1] = { nop };
