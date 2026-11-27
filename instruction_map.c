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

uint8_t getZ(State* s){
    return ((*s->cpu->AF) & (1 << 7)) >> 7;
}

uint8_t getSubtraction(State* s){
    return ((*s->cpu->AF) & (1 << 6)) >> 6;
}

uint8_t getHalfCarry(State* s){
    return ((*s->cpu->AF) & (1 << 5)) >> 5;
}

uint8_t getCarry(State* s){
    return ((*s->cpu->AF) & (1 << 4)) >> 4;
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

void ld_bc_d16(State* state){
    *state->cpu->BC = read_d16(state);
    *state->cpu->PC += 3;
    sleep_cycles(state, 3);
}

void ldbc_a(State* state){
    mm_write(state, *state->cpu->BC, *state->cpu->A);
    (*state->cpu->PC)++;

    sleep_cycles(state, 2);
}

void inc_bc(State* state){
    (*state->cpu->BC)++;
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void inc_u8(State* s, uint8_t* v){
    setSubtraction(s, 0);
    if (*v == 0xFF){
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
    if (*v == 0x00){
        setHalfCarry(s, 1);
        setZ(s, 0);
    } else if (*v == 0x01){
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

void sub_u8(State* state, uint8_t* dst, uint8_t* op){
    setSubtraction(state, 1);
    if ((((*dst) & 0x0F) - ((*op) & 0x0F)) == 0x10) {
        setHalfCarry(state, 1);
    } else {
        setHalfCarry(state, 0);
    }
    
    uint16_t d = *dst;
    uint16_t o = *op;
    if (((d - o) & 0x100) == 0x100) {
        setCarry(state, 1);
    } else {
        setCarry(state, 0);
    }

    *dst -= *op;

    if (*dst == 0){
        setZ(state, 1);
    } else {
        setZ(state, 0);
    }
}

void sbc_u8(State* state, uint8_t* dst, uint8_t* op){
    uint8_t intermediary = *op;
    intermediary += getCarry(state);
    sub_u8(state, dst, &intermediary);
}

void add_u8(State* state, uint8_t* dst, uint8_t* op){
    setSubtraction(state, 0);
    if ((((*dst) & 0x0F) + ((*op) & 0x0F) & 0x10) == 0x10){
        setHalfCarry(state, 1);
    } else {
        setHalfCarry(state, 0);
    }

    // kinda scuffed way to calculate carry, but it works
    uint16_t d = *dst;
    uint16_t o = *op;
    if (((d + o) & 0x100) == 0x100) {
        setCarry(state, 1);
    } else {
        setCarry(state, 0);
    }

    *dst += *op;

    if (*dst == 0){
        setZ(state, 1);
    } else {
        setZ(state, 0);
    }
}

void adc_u8(State* state, uint8_t* dst, uint8_t* op){
    uint8_t intermediary = *op;
    intermediary += getCarry(state);
    add_u8(state, dst, &intermediary);
}

void add_u16(State* state, uint16_t* dst, uint16_t* op){
    setSubtraction(state, 0);
    if ((((*dst) & 0x0F) + ((*op) & 0x0F) & 0x10) == 0x10){
        setHalfCarry(state, 1);
    } else {
        setHalfCarry(state, 0);
    }

    if ((((*dst) & 0xFF) + ((*op) & 0xFF) & 0x100) == 0x100){
        setCarry(state, 1);
    } else {
        setCarry(state, 0);
    }

    *dst += *op;
    if (*dst == 0){
        setZ(state, 1);
    } else {
        setZ(state, 0);
    }
}

void sub_u16(State* state, uint16_t* dst, uint16_t* op){
    setSubtraction(state, 1);
    if ((((*dst) & 0x0F) - ((*op) & 0x0F) & 0x10) == 0x10){
        setHalfCarry(state, 1);
    } else {
        setHalfCarry(state, 0);
    }

    if ((((*dst) & 0xFF) - ((*op) & 0xFF) & 0x100) == 0x100){
        setCarry(state, 1);
    } else {
        setCarry(state, 0);
    }

    *dst -= *op;
    if (*dst == 0){
        setZ(state, 1);
    } else {
        setZ(state, 0);
    }
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

void inc_c(State* state){
    inc_u8(state, state->cpu->C);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void dec_c(State* state){
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
    if (state->debug_mode){
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
    // I believe the address is relative to the end of the jr instruction
    (*state->cpu->PC) += (d + 0x2);
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

void rra(State* state){
    rotr(state, state->cpu->A, 0);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void jr_nz_s8(State* state){
    int8_t d = (int8_t)read_d8(state);
    if (getZ(state) == 0){
        (*state->cpu->PC) += (d + 0x02);
        sleep_cycles(state, 3);
    } else {
        (*state->cpu->PC) += 2;
        sleep_cycles(state, 2);
    }
}

void ld_hl_d16(State* state){
    (*state->cpu->HL) = read_d16(state);
    (*state->cpu->PC) += 3;
    sleep_cycles(state, 3);
}

void ld_hlp_a(State* state){
    mm_write(state, (*state->cpu->HL)++, *state->cpu->A);
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void inc_hl(State* state){
    (*state->cpu->HL)++;
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void inc_h(State* state){
    inc_u8(state, state->cpu->H);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void dec_h(State* state){
    dec_u8(state, state->cpu->H);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void ld_h_d8(State* state){
    (*state->cpu->H) = read_d8(state);
    (*state->cpu->PC) += 2;
    sleep_cycles(state, 2);
}

void daa(State* state){ // very scary instruction
    uint8_t op = (getSubtraction(state)) ? (-1) : (1); // no repeated code in my codebase >:3
    if (getHalfCarry(state)){
        *state->cpu->A += (op * 0x06);
    }
    
    setHalfCarry(state, 0);

    if (getCarry(state)){
        *state->cpu->A += (op * 0x60);
    }
    
    if (*state->cpu->A > 0x99) {
        setCarry(state, 1);
        *state->cpu->A += (op * 0x60);
    } else {
        setCarry(state, 0);
    }

    if (*state->cpu->A == 0x0){
        setZ(state, 1);
    } else {
        setZ(state, 0);
    }
}

void jr_z_s8(State* state){
    if (getZ(state)){
        int8_t d = read_d8(state);
        (*state->cpu->PC) += (d + 0x02);
        sleep_cycles(state, 3);
    } else {
        (*state->cpu->PC) += 2;
        sleep_cycles(state, 2);
    }
}

void add_hl_hl(State* state){
    add_u16(state, state->cpu->HL, state->cpu->HL);
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void ld_a_hlp(State* state){
    *state->cpu->A = mm_read(state, (*state->cpu->HL)++);
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void dec_hl(State* state){
    (*state->cpu->HL)--;
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void inc_l(State* state){
    inc_u8(state, state->cpu->L);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void dec_l(State* state){
    dec_u8(state, state->cpu->L);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void ld_l_d8(State* state){
    (*state->cpu->L) = read_d8(state);
    (*state->cpu->PC) += 2;
    sleep_cycles(state, 2);
}

void cpl(State* state){
    setSubtraction(state, 1);
    setHalfCarry(state, 1);
    (*state->cpu->A) = ~(*state->cpu->A);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void jr_nc_s8(State* state){
    if (getCarry(state) == 0){
        int8_t d = (int8_t)read_d8(state);
        (*state->cpu->PC) += (d + 2);
        sleep_cycles(state, 3);
    } else {
        (*state->cpu->PC) += 2;
        sleep_cycles(state, 2);
    }
}

void ld_sp_d16(State* state){
    (*state->cpu->SP) = read_d16(state);
    (*state->cpu->PC) += 3;
    sleep_cycles(state, 3);
}

void ld_hlm_a(State* state){
    mm_write(state, (*state->cpu->HL)--, *state->cpu->A);
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void inc_sp(State* state){
    (*state->cpu->SP)++;
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void inchl(State* state){
    // kinda funky shenanigans because I can't take a reference to the return value of mm_read
    uint8_t v = mm_read(state, *state->cpu->HL);
    inc_u8(state, &v);
    mm_write(state, *state->cpu->HL, v);
    (*state->cpu->PC)++;
    sleep_cycles(state, 3);
}

void dechl(State* state){
    // same as above
    uint8_t v = mm_read(state, *state->cpu->HL);
    dec_u8(state, &v);
    mm_write(state, *state->cpu->HL, v);
    (*state->cpu->PC)++;
    sleep_cycles(state, 3);
}

void ldhl_d8(State* state){
    mm_write(state, *state->cpu->HL, read_d8(state));
    (*state->cpu->PC) += 2;
    sleep_cycles(state, 3);
}

void scf(State* state){
    setCarry(state, 1);
    setHalfCarry(state, 0);
    setSubtraction(state, 0);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void jr_c_s8(State* state){
    if (getCarry(state)) {
        int8_t d = (int8_t)read_d8(state);
        (*state->cpu->PC) += (d + 2);
        sleep_cycles(state, 3);
    } else {
        (*state->cpu->PC) += 2;
        sleep_cycles(state, 2);
    }
}

void add_hl_sp(State* state){
    add_u16(state, state->cpu->HL, state->cpu->SP);
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void ld_a_hlm(State* state){
    (*state->cpu->A) = mm_read(state, (*state->cpu->HL)--);   
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void dec_sp(State* state){
    (*state->cpu->SP)--;
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void inc_a(State* state){
    inc_u8(state, state->cpu->A);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void dec_a(State* state){
    dec_u8(state, state->cpu->A);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void ld_a_d8(State* state){
    (*state->cpu->A) = read_d8(state);
    (*state->cpu->PC) += 2;
    sleep_cycles(state, 2);
}

void ccf(State* state){
    setCarry(state, !getCarry(state));
    setHalfCarry(state, 0);
    setSubtraction(state, 0);
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void ld_a_a(State* state){
	(*state->cpu->A) = (*state->cpu->A);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_a_b(State* state){
	(*state->cpu->A) = (*state->cpu->B);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_a_c(State* state){
	(*state->cpu->A) = (*state->cpu->C);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_a_d(State* state){
	(*state->cpu->A) = (*state->cpu->D);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_a_e(State* state){
	(*state->cpu->A) = (*state->cpu->E);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_a_h(State* state){
	(*state->cpu->A) = (*state->cpu->H);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_a_l(State* state){
	(*state->cpu->A) = (*state->cpu->L);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_ahl(State* state){
	(*state->cpu->A) = mm_read(state, *state->cpu->HL);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void ld_b_a(State* state){
	(*state->cpu->B) = (*state->cpu->A);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_b_b(State* state){
	(*state->cpu->B) = (*state->cpu->B);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_b_c(State* state){
	(*state->cpu->B) = (*state->cpu->C);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_b_d(State* state){
	(*state->cpu->B) = (*state->cpu->D);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_b_e(State* state){
	(*state->cpu->B) = (*state->cpu->E);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_b_h(State* state){
	(*state->cpu->B) = (*state->cpu->H);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_b_l(State* state){
	(*state->cpu->B) = (*state->cpu->L);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_bhl(State* state){
	(*state->cpu->B) = mm_read(state, *state->cpu->HL);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void ld_c_a(State* state){
	(*state->cpu->C) = (*state->cpu->A);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_c_b(State* state){
	(*state->cpu->C) = (*state->cpu->B);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_c_c(State* state){
	(*state->cpu->C) = (*state->cpu->C);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_c_d(State* state){
	(*state->cpu->C) = (*state->cpu->D);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_c_e(State* state){
	(*state->cpu->C) = (*state->cpu->E);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_c_h(State* state){
	(*state->cpu->C) = (*state->cpu->H);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_c_l(State* state){
	(*state->cpu->C) = (*state->cpu->L);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_chl(State* state){
	(*state->cpu->C) = mm_read(state, *state->cpu->HL);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void ld_d_a(State* state){
	(*state->cpu->D) = (*state->cpu->A);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_d_b(State* state){
	(*state->cpu->D) = (*state->cpu->B);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_d_c(State* state){
	(*state->cpu->D) = (*state->cpu->C);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_d_d(State* state){
	(*state->cpu->D) = (*state->cpu->D);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_d_e(State* state){
	(*state->cpu->D) = (*state->cpu->E);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_d_h(State* state){
	(*state->cpu->D) = (*state->cpu->H);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_d_l(State* state){
	(*state->cpu->D) = (*state->cpu->L);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_dhl(State* state){
	(*state->cpu->D) = mm_read(state, *state->cpu->HL);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void ld_e_a(State* state){
	(*state->cpu->E) = (*state->cpu->A);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_e_b(State* state){
	(*state->cpu->E) = (*state->cpu->B);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_e_c(State* state){
	(*state->cpu->E) = (*state->cpu->C);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_e_d(State* state){
	(*state->cpu->E) = (*state->cpu->D);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_e_e(State* state){
	(*state->cpu->E) = (*state->cpu->E);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_e_h(State* state){
	(*state->cpu->E) = (*state->cpu->H);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_e_l(State* state){
	(*state->cpu->E) = (*state->cpu->L);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_ehl(State* state){
	(*state->cpu->E) = mm_read(state, *state->cpu->HL);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void ld_h_a(State* state){
	(*state->cpu->H) = (*state->cpu->A);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_h_b(State* state){
	(*state->cpu->H) = (*state->cpu->B);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_h_c(State* state){
	(*state->cpu->H) = (*state->cpu->C);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_h_d(State* state){
	(*state->cpu->H) = (*state->cpu->D);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_h_e(State* state){
	(*state->cpu->H) = (*state->cpu->E);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_h_h(State* state){
	(*state->cpu->H) = (*state->cpu->H);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_h_l(State* state){
	(*state->cpu->H) = (*state->cpu->L);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_hhl(State* state){
	(*state->cpu->H) = mm_read(state, *state->cpu->HL);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void ld_l_a(State* state){
	(*state->cpu->L) = (*state->cpu->A);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_l_b(State* state){
	(*state->cpu->L) = (*state->cpu->B);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_l_c(State* state){
	(*state->cpu->L) = (*state->cpu->C);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_l_d(State* state){
	(*state->cpu->L) = (*state->cpu->D);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_l_e(State* state){
	(*state->cpu->L) = (*state->cpu->E);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_l_h(State* state){
	(*state->cpu->L) = (*state->cpu->H);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_l_l(State* state){
	(*state->cpu->L) = (*state->cpu->L);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void ld_lhl(State* state){
	(*state->cpu->L) = mm_read(state, *state->cpu->HL);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void ldhl_a(State* state){
	mm_write(state, *state->cpu->HL, *state->cpu->A);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void ldhl_b(State* state){
	mm_write(state, *state->cpu->HL, *state->cpu->B);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void ldhl_c(State* state){
	mm_write(state, *state->cpu->HL, *state->cpu->C);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void ldhl_d(State* state){
	mm_write(state, *state->cpu->HL, *state->cpu->D);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void ldhl_e(State* state){
	mm_write(state, *state->cpu->HL, *state->cpu->E);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void ldhl_h(State* state){
	mm_write(state, *state->cpu->HL, *state->cpu->H);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void ldhl_l(State* state){
	mm_write(state, *state->cpu->HL, *state->cpu->L);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void halt(State* state){
	printf("WARN: reached unimplemented instruction 'HALT', skipping...\n");
}

void add_a_a(State* state){
	add_u8(state, state->cpu->A, state->cpu->A);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void add_a_b(State* state){
	add_u8(state, state->cpu->A, state->cpu->B);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void add_a_c(State* state){
	add_u8(state, state->cpu->A, state->cpu->C);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void add_a_d(State* state){
	add_u8(state, state->cpu->A, state->cpu->D);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void add_a_e(State* state){
	add_u8(state, state->cpu->A, state->cpu->E);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void add_a_h(State* state){
	add_u8(state, state->cpu->A, state->cpu->H);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void add_a_l(State* state){
	add_u8(state, state->cpu->A, state->cpu->L);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void add_ahl(State* state){
	uint8_t v = mm_read(state, *state->cpu->HL);
	add_u8(state, state->cpu->A, &v);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void adc_a_a(State* state){
	adc_u8(state, state->cpu->A, state->cpu->A);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void adc_a_b(State* state){
	adc_u8(state, state->cpu->A, state->cpu->B);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void adc_a_c(State* state){
	adc_u8(state, state->cpu->A, state->cpu->C);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void adc_a_d(State* state){
	adc_u8(state, state->cpu->A, state->cpu->D);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void adc_a_e(State* state){
	adc_u8(state, state->cpu->A, state->cpu->E);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void adc_a_h(State* state){
	adc_u8(state, state->cpu->A, state->cpu->H);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void adc_a_l(State* state){
	adc_u8(state, state->cpu->A, state->cpu->L);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void adc_ahl(State* state){
	uint8_t v = mm_read(state, *state->cpu->HL);
	adc_u8(state, state->cpu->A, &v);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void sub_a_a(State* state){
	sub_u8(state, state->cpu->A, state->cpu->A);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void sub_a_b(State* state){
	sub_u8(state, state->cpu->A, state->cpu->B);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void sub_a_c(State* state){
	sub_u8(state, state->cpu->A, state->cpu->C);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void sub_a_d(State* state){
	sub_u8(state, state->cpu->A, state->cpu->D);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void sub_a_e(State* state){
	sub_u8(state, state->cpu->A, state->cpu->E);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void sub_a_h(State* state){
	sub_u8(state, state->cpu->A, state->cpu->H);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void sub_a_l(State* state){
	sub_u8(state, state->cpu->A, state->cpu->L);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void sub_ahl(State* state){
	uint8_t v = mm_read(state, *state->cpu->HL);
	sub_u8(state, state->cpu->A, &v);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void sbc_a_a(State* state){
	sbc_u8(state, state->cpu->A, state->cpu->A);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void sbc_a_b(State* state){
	sbc_u8(state, state->cpu->A, state->cpu->B);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void sbc_a_c(State* state){
	sbc_u8(state, state->cpu->A, state->cpu->C);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void sbc_a_d(State* state){
	sbc_u8(state, state->cpu->A, state->cpu->D);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void sbc_a_e(State* state){
	sbc_u8(state, state->cpu->A, state->cpu->E);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void sbc_a_h(State* state){
	sbc_u8(state, state->cpu->A, state->cpu->H);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void sbc_a_l(State* state){
	sbc_u8(state, state->cpu->A, state->cpu->L);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void sbc_ahl(State* state){
	uint8_t v = mm_read(state, *state->cpu->HL);
	sbc_u8(state, state->cpu->A, &v);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void and_a(State* state){
	setHalfCarry(state, 1);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) &= (*state->cpu->A);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void and_b(State* state){
	setHalfCarry(state, 1);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) &= (*state->cpu->B);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void and_c(State* state){
	setHalfCarry(state, 1);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) &= (*state->cpu->C);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void and_d(State* state){
	setHalfCarry(state, 1);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) &= (*state->cpu->D);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void and_e(State* state){
	setHalfCarry(state, 1);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) &= (*state->cpu->E);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void and_h(State* state){
	setHalfCarry(state, 1);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) &= (*state->cpu->H);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void and_l(State* state){
	setHalfCarry(state, 1);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) &= (*state->cpu->L);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void and_hl(State* state){
	setHalfCarry(state, 1);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) &= mm_read(state, *state->cpu->HL);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void xor_a(State* state){
	setHalfCarry(state, 0);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) ^= (*state->cpu->A);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void xor_b(State* state){
	setHalfCarry(state, 0);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) ^= (*state->cpu->B);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void xor_c(State* state){
	setHalfCarry(state, 0);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) ^= (*state->cpu->C);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void xor_d(State* state){
	setHalfCarry(state, 0);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) ^= (*state->cpu->D);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void xor_e(State* state){
	setHalfCarry(state, 0);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) ^= (*state->cpu->E);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void xor_h(State* state){
	setHalfCarry(state, 0);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) ^= (*state->cpu->H);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void xor_l(State* state){
	setHalfCarry(state, 0);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) ^= (*state->cpu->L);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void xor_hl(State* state){
	setHalfCarry(state, 0);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) ^= mm_read(state, *state->cpu->HL);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void or_a(State* state){
	setHalfCarry(state, 0);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) |= (*state->cpu->A);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void or_b(State* state){
	setHalfCarry(state, 0);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) |= (*state->cpu->B);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void or_c(State* state){
	setHalfCarry(state, 0);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) |= (*state->cpu->C);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void or_d(State* state){
	setHalfCarry(state, 0);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) |= (*state->cpu->D);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void or_e(State* state){
	setHalfCarry(state, 0);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) |= (*state->cpu->E);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void or_h(State* state){
	setHalfCarry(state, 0);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) |= (*state->cpu->H);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void or_l(State* state){
	setHalfCarry(state, 0);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) |= (*state->cpu->L);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void or_hl(State* state){
	setHalfCarry(state, 0);
	setCarry(state, 0);
	setSubtraction(state, 0);
	(*state->cpu->A) |= mm_read(state, *state->cpu->HL);
	if ((*state->cpu->A) == 0) {
		setZ(state, 1);
	} else {
		setZ(state, 0);
	}	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void cp_a(State* state){
	uint8_t a = (*state->cpu->A);
	sub_u8(state, &a, state->cpu->A);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void cp_b(State* state){
	uint8_t a = (*state->cpu->A);
	sub_u8(state, &a, state->cpu->B);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void cp_c(State* state){
	uint8_t a = (*state->cpu->A);
	sub_u8(state, &a, state->cpu->C);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void cp_d(State* state){
	uint8_t a = (*state->cpu->A);
	sub_u8(state, &a, state->cpu->D);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void cp_e(State* state){
	uint8_t a = (*state->cpu->A);
	sub_u8(state, &a, state->cpu->E);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void cp_h(State* state){
	uint8_t a = (*state->cpu->A);
	sub_u8(state, &a, state->cpu->H);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void cp_l(State* state){
	uint8_t a = (*state->cpu->A);
	sub_u8(state, &a, state->cpu->L);
	(*state->cpu->PC)++;
	sleep_cycle(state);
}

void cphl(State* state){
	uint8_t a = (*state->cpu->A);
	uint8_t hl = (*state->cpu->HL);
	sub_u8(state, &a, &hl);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

uint16_t pop(State* s){
    return ((uint16_t)mm_read(s, (*s->cpu->SP)++) |
            ((uint16_t)mm_read(s, (*s->cpu->SP)++) << 8));
}

void push_u8(State* s, uint8_t v){
    mm_write(s, --(*s->cpu->SP), v);
}

void push_u16(State* s, uint16_t v){
    push_u8(s, (v >> 8) & 0xFF);
    push_u8(s, (v & 0xFF));
}

void ret_nz(State* state){
    if (getZ(state) != 0){
        (*state->cpu->PC)++;
        sleep_cycles(state, 2);
    } else {
        *state->cpu->PC = pop(state);
        sleep_cycles(state, 5);
    }
}

void pop_bc(State* state){
    *state->cpu->BC = pop(state);
    (*state->cpu->PC)++;
    sleep_cycles(state, 3);
}

void jp_nz_a16(State* state){
    if (getZ(state) != 0){
        (*state->cpu->PC) += 3;
        sleep_cycles(state, 3);
    } else {
        (*state->cpu->PC) = read_d16(state);
        sleep_cycles(state, 4);
    }
}

void jp_a16(State* state){
    (*state->cpu->PC) = read_d16(state);
    sleep_cycles(state, 4);
}

void call_nz_a16(State* state){
    if (getZ(state) != 0){
        (*state->cpu->PC) += 3;
        sleep_cycles(state, 3);
    } else {
        push_u16(state, (*state->cpu->PC) + 3);
        (*state->cpu->PC) += read_d16(state);
        sleep_cycles(state, 6);
    }
}

void push_bc(State* state){
    push_u16(state, *state->cpu->BC);
    (*state->cpu->PC)++;
    sleep_cycles(state, 4);
}

void add_a_d8(State* state){
    uint8_t v = read_d8(state);
    add_u8(state, state->cpu->A, &v);
    (*state->cpu->PC) += 2;
    sleep_cycles(state, 2);
}

void rst_0(State* state){
    push_u16(state, *state->cpu->PC);
    (*state->cpu->PC) = 0x00;
    sleep_cycles(state, 4);
}

void ret_z(State* state){
    if (getZ(state) == 0){
        (*state->cpu->PC)++;
        sleep_cycles(state, 2);
    } else {
        (*state->cpu->PC) = pop(state);
        sleep_cycles(state, 5);
    }
}

void ret(State* state){
    (*state->cpu->PC) = pop(state);
    sleep_cycles(state, 4);
}

void jp_z_a16(State* state){
    if (getZ(state) == 0){
        (*state->cpu->PC) += 3;
        sleep_cycles(state, 3);
    } else {
        (*state->cpu->PC) = read_d16(state);
        sleep_cycles(state, 4);
    }
}

void unk(State* state){
    printf("WARN: reached unknown instruction 0x%X\n", mm_read(state, *state->cpu->PC));
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void call_z_a16(State* state){
    if (getZ(state) == 0){
        (*state->cpu->PC) += 3;
        sleep_cycles(state, 3);
    } else {
        push_u16(state, *state->cpu->PC);
        (*state->cpu->PC) = read_d16(state);
        sleep_cycles(state, 6);
    }
}

void call_a16(State* state){
    push_u16(state, *state->cpu->PC);
    (*state->cpu->PC) = read_d16(state);
    sleep_cycles(state, 6);
}

void adc_a_d8(State* state){
    uint8_t v = read_d8(state);
    adc_u8(state, state->cpu->A, &v);
    (*state->cpu->PC) += 2;
    sleep_cycles(state, 2);
}

void rst_1(State* state){
    push_u16(state, *state->cpu->PC);
    (*state->cpu->PC) = 0x08;
    sleep_cycles(state, 4);
}

void ret_nc(State* state){
    if (getCarry(state) != 0){
        (*state->cpu->PC)++;
        sleep_cycles(state, 2);
    } else {
        (*state->cpu->PC) = pop(state);
        sleep_cycles(state, 5);
    }
}

void pop_de(State* state){
    (*state->cpu->DE) = pop(state);
    (*state->cpu->PC)++;
    sleep_cycles(state, 3);
}

void jp_nc_a16(State* state){
    if (getCarry(state) != 0){
        (*state->cpu->PC) += 3;
        sleep_cycles(state, 3);
    } else {
        (*state->cpu->PC) = read_d16(state);
        sleep_cycles(state, 4);
    }
}

void call_nc_a16(State* state){
    if (getCarry(state) != 0){
        (*state->cpu->PC) += 3;
        sleep_cycles(state, 3);
    } else {
        push_u16(state, *state->cpu->PC);
        (*state->cpu->PC) = read_d16(state);
        sleep_cycles(state, 6);
    }
}

void push_de(State* state){
    push_u16(state, *state->cpu->DE);
    (*state->cpu->PC)++;
    sleep_cycles(state, 4);
}

void sub_d8(State* state){
    uint8_t v = read_d8(state);
    sub_u8(state, state->cpu->A, &v);
    (*state->cpu->PC) += 2;
    sleep_cycles(state, 2);
}

void rst_2(State* state){
    push_u16(state, *state->cpu->PC);
    (*state->cpu->PC) = 0x10;
    sleep_cycles(state, 4);
}

void ret_c(State* state){
    if (getCarry(state) == 0){
        (*state->cpu->PC)++;
        sleep_cycles(state, 2);
    } else {
        (*state->cpu->PC) = pop(state);
        sleep_cycles(state, 5);
    }
}

void reti(State* state){
    (*state->cpu->PC) = pop(state);
    sleep_cycles(state, 4);
    state->should_enable_interrupts = 1;
}

void jp_c_a16(State* state){
    if (getCarry(state) == 0){
        (*state->cpu->PC) += 3;
        sleep_cycles(state, 3);
    } else {
        (*state->cpu->PC) = read_d16(state);
        sleep_cycles(state, 4);
    }
}

void call_c_a16(State* state){
    if (getCarry(state) == 0){
        (*state->cpu->PC) += 3;
        sleep_cycles(state, 3);
    } else {
        push_u16(state, (*state->cpu->PC));
        (*state->cpu->PC) = read_d16(state);
        sleep_cycles(state, 6);
    }
}

void sbc_a_d8(State* state){
    uint8_t intermediary = read_d8(state);
    intermediary += getCarry(state);
    sbc_u8(state, state->cpu->A, &intermediary);
    (*state->cpu->PC) += 2;
    sleep_cycles(state, 2);
}

void rst_3(State* state){
    push_u16(state, *state->cpu->PC);
    (*state->cpu->PC) = 0x18;
    sleep_cycles(state, 4);
}

void lda8_a(State* state){
    mm_write(state, read_d8(state) + 0xFF00, (*state->cpu->A));
    (*state->cpu->PC) += 2;
    sleep_cycles(state, 3);
}

void pop_hl(State* state){
    (*state->cpu->HL) = pop(state);
    (*state->cpu->PC)++;
    sleep_cycles(state, 3);
}

void ldc_a(State* state){
    mm_write(state, (*state->cpu->C) + 0xFF00, (*state->cpu->A));
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void push_hl(State* state){
    push_u16(state, *state->cpu->HL);
    (*state->cpu->PC)++;
    sleep_cycles(state, 4);
}

void and_d8(State* state){
   setSubtraction(state, 0);
   setCarry(state, 0);
   setHalfCarry(state, 1);
   (*state->cpu->A) &= read_d8(state);
   if (*state->cpu->A == 0){
        setZ(state, 1);
   } else {
        setZ(state, 0);
   }

   (*state->cpu->PC) += 2;
   sleep_cycles(state, 2);
}

void rst_4(State* state){
    push_u16(state, *state->cpu->PC);
    (*state->cpu->PC) = 0x20;
    sleep_cycles(state, 4);
}

void add_sp_s8(State* state){
    int16_t d = (int8_t)read_d8(state);
    if (d < 0){
        d = -d;
        sub_u16(state, state->cpu->SP, (uint16_t*)&d);
    } else {
        add_u16(state, state->cpu->SP, (uint16_t*)&d);
    }

    setZ(state, 0);
    setSubtraction(state, 0);
    (*state->cpu->PC) += 2;
    sleep_cycles(state, 4);
}

void jp_hl(State* state){
    (*state->cpu->PC) = (*state->cpu->HL);
    sleep_cycle(state);
}

void lda16_a(State* state){
    mm_write(state, read_d16(state), *state->cpu->A);
    (*state->cpu->PC) += 3;
    sleep_cycles(state, 4);
}

void xor_d8(State* state){
    (*state->cpu->A) ^= read_d8(state);
    setSubtraction(state, 0);
    setHalfCarry(state, 0);
    setCarry(state, 0);
    if (*state->cpu->A == 0){
        setZ(state, 1);
    } else {
        setZ(state, 0);
    }
}

void rst_5(State* state){
    push_u16(state, *state->cpu->PC);
    (*state->cpu->PC) = 0x28;
    sleep_cycles(state, 4);
}

void ld_aa8(State* state){
    (*state->cpu->A) = mm_read(state, read_d8(state) + 0xFF00);
    (*state->cpu->PC) += 2;
    sleep_cycles(state, 3);
}

void pop_af(State* state){
    (*state->cpu->AF) = pop(state);
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void ld_ac(State* state){
    (*state->cpu->A) = mm_read(state, (*state->cpu->C) + 0xFF00);
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void di(State* state){
    state->cpu->IME = 0;
    (*state->cpu->PC)++;
    sleep_cycle(state);
}

void push_af(State* state){
    push_u16(state, *state->cpu->AF);
    (*state->cpu->PC)++;
    sleep_cycles(state, 4);
}

void or_d8(State* state){
    setSubtraction(state, 0);
    setHalfCarry(state, 0);
    setCarry(state, 0);

    (*state->cpu->A) |= read_d8(state);
    if (*state->cpu->A == 0){
        setZ(state, 1);
    } else {
        setZ(state, 0);
    }
}

void rst_6(State* state){
    push_u16(state, *state->cpu->PC);
    *state->cpu->PC = 0x30;
    sleep_cycles(state, 4);
}

void ld_hl_sp_s8(State* state){
    int16_t d = (int8_t)read_d8(state);
    uint16_t sp = *state->cpu->SP;

    if (d < 0){
        d = -d;
        sub_u16(state, &sp, (uint16_t*)&d);
    } else {
        add_u16(state, &sp, (uint16_t*)&d);
    }
    
    setZ(state, 0);
    setSubtraction(state, 0);   
    (*state->cpu->HL) = sp;
    (*state->cpu->PC) += 2;
    sleep_cycles(state, 3);
}

void ld_sp_hl(State* state){
    (*state->cpu->SP) = (*state->cpu->HL);
    (*state->cpu->PC)++;
    sleep_cycles(state, 2);
}

void ld_aa16(State* state){
    (*state->cpu->A) = mm_read(state, read_d16(state));
    (*state->cpu->PC) += 3;
    sleep_cycles(state, 4);
}

void ei(State* state){
    (*state->cpu->PC)++;
    sleep_cycle(state);
    state->should_enable_interrupts = 1;
}

void cp_d8(State* state){
    uint8_t a = (*state->cpu->A);
    uint8_t b = read_d8(state);
    sub_u8(state, &a, &b);
    (*state->cpu->PC) += 2;
    sleep_cycles(state, 2);
}

void rst_7(State* state){
    push_u16(state, *state->cpu->PC);
    (*state->cpu->PC) = 0x38;
    sleep_cycles(state, 4);
}

InstructionFn instruction_map[0x100] = {
    nop,      ld_bc_d16, ldbc_a,    inc_bc,  inc_b,      dec_b,   ld_b_d8,  rlca,    lda16_sp, add_hl_bc, ld_a_bc,  dec_bc,   inc_c,   dec_c,   ld_c_d8, rrca,
    stop,     ld_de_d16, ldde_a,    inc_de,  inc_d,      dec_d,   ld_d_d8,  rla,     jr_s8,    add_hl_de, ld_a_de,  dec_de,   inc_e,   dec_e,   ld_e_d8, rra,
    jr_nz_s8, ld_hl_d16, ld_hlp_a,  inc_hl,  inc_h,      dec_h,   ld_h_d8,  daa,     jr_z_s8,  add_hl_hl, ld_a_hlp, dec_hl,   inc_l,   dec_l,   ld_l_d8, cpl,
    jr_nc_s8, ld_sp_d16, ld_hlm_a,  inc_sp,  inchl,      dechl,   ldhl_d8,  scf,     jr_c_s8,  add_hl_sp, ld_a_hlm, dec_sp,   inc_a,   dec_a,   ld_a_d8, ccf,
    ld_b_b,   ld_b_c,    ld_b_d,    ld_b_e,  ld_b_h,     ld_b_l,  ld_bhl,   ld_b_a,  ld_c_b,   ld_c_c,    ld_c_d,   ld_c_e,   ld_c_h,  ld_c_l,  ld_chl,  ld_c_a,
    ld_d_b,   ld_d_c,    ld_d_d,    ld_d_e,  ld_d_h,     ld_d_l,  ld_dhl,   ld_d_a,  ld_e_b,   ld_e_c,    ld_e_d,   ld_e_e,   ld_e_h,  ld_e_l,  ld_ehl,  ld_e_a,
    ld_h_b,   ld_h_c,    ld_h_d,    ld_h_e,  ld_h_h,     ld_h_l,  ld_hhl,   ld_h_a,  ld_l_b,   ld_l_c,    ld_l_d,   ld_l_e,   ld_l_h,  ld_l_l,  ld_lhl,  ld_l_a,
    ldhl_b,   ldhl_c,    ldhl_d,    ldhl_e,  ldhl_h,     ldhl_l,  halt,     ldhl_a,  ld_a_b,   ld_a_c,    ld_a_d,   ld_a_e,   ld_a_h,  ld_a_l,  ld_ahl,  ld_a_a,
    add_a_b,  add_a_c,   add_a_d,   add_a_e, add_a_h,    add_a_l, add_ahl,  add_a_a, adc_a_b,  adc_a_c,   adc_a_d,  adc_a_e,  adc_a_h, adc_a_l, adc_ahl, adc_a_a,
    sub_a_b,  sub_a_c,   sub_a_d,   sub_a_e, sub_a_h,    sub_a_l, sub_ahl,  sub_a_a, sbc_a_b,  sbc_a_c,   sbc_a_d,  sbc_a_e,  sbc_a_h, sbc_a_l, sbc_ahl, sbc_a_a,
    and_b,    and_c,     and_d,     and_e,   and_h,      and_l,   and_hl,   and_a,   xor_b,    xor_c,     xor_d,    xor_e,    xor_h,   xor_l,   xor_hl,  xor_a,
    or_b,     or_c,      or_d,      or_e,    or_h,       or_l,    or_hl,    or_a,    cp_b,     cp_c,      cp_d,     cp_e,     cp_h,    cp_l,    cphl,    cp_a,
    ret_nz,   pop_bc,    jp_nz_a16, jp_a16,  call_nz_a16, push_bc, add_a_d8, rst_0,  ret_z,    ret,       jp_z_a16, unk, call_z_a16, call_a16, adc_a_d8, rst_1,
    ret_nc,   pop_de,    jp_nc_a16, unk,     call_nc_a16, push_de, sub_d8,  rst_2,   ret_c,    reti,      jp_c_a16, unk, call_c_a16, unk, sbc_a_d8, rst_3,
    lda8_a,   pop_hl,    ldc_a,     unk,     unk,         push_hl, and_d8,  rst_4,   add_sp_s8, jp_hl,    lda16_a,  unk, unk,        unk, xor_d8,   rst_5,
    ld_aa8,   pop_af,    ld_ac,     di,      unk,         push_af, or_d8,   rst_6,   ld_hl_sp_s8, ld_sp_hl, ld_aa16, ei, unk,        unk, cp_d8,    rst_7
};

void lshift(State* state, uint8_t* v){
    setCarry(state, ((*v) & 0x80) >> 7);
    *v = (*v) << 1;
}

void rshift(State* state, uint8_t* v){
    setCarry(state, (*v) & 0x01);
    *v = (*v) >> 1;
}

void rshift_retain_b7(State* state, uint8_t* v){
    uint8_t b7 = (*v) & 0x80;
    setCarry(state, (*v) & 0x01);
    *v = (((*v) >> 1) | b7);
}

void swp(State* state, uint8_t* v){
    uint8_t l = ((*v) & 0x0F);
    uint8_t h = ((*v) & 0xF0);
    *v = ((l << 4) | (h >> 4));
}

void rlc_a(State* state){
	(*state->cpu->PC)++;
	rotlc(state, (state->cpu->A), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rlc_b(State* state){
	(*state->cpu->PC)++;
	rotlc(state, (state->cpu->B), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rlc_c(State* state){
	(*state->cpu->PC)++;
	rotlc(state, (state->cpu->C), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rlc_d(State* state){
	(*state->cpu->PC)++;
	rotlc(state, (state->cpu->D), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rlc_e(State* state){
	(*state->cpu->PC)++;
	rotlc(state, (state->cpu->E), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rlc_h(State* state){
	(*state->cpu->PC)++;
	rotlc(state, (state->cpu->H), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rlc_l(State* state){
	(*state->cpu->PC)++;
	rotlc(state, (state->cpu->L), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rlchl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	rotlc(state, &a, 1);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void rrc_a(State* state){
	(*state->cpu->PC)++;
	rotrc(state, (state->cpu->A), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rrc_b(State* state){
	(*state->cpu->PC)++;
	rotrc(state, (state->cpu->B), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rrc_c(State* state){
	(*state->cpu->PC)++;
	rotrc(state, (state->cpu->C), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rrc_d(State* state){
	(*state->cpu->PC)++;
	rotrc(state, (state->cpu->D), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rrc_e(State* state){
	(*state->cpu->PC)++;
	rotrc(state, (state->cpu->E), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rrc_h(State* state){
	(*state->cpu->PC)++;
	rotrc(state, (state->cpu->H), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rrc_l(State* state){
	(*state->cpu->PC)++;
	rotrc(state, (state->cpu->L), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rrchl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	rotrc(state, &a, 1);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void rl_a(State* state){
	(*state->cpu->PC)++;
	rotl(state, (state->cpu->A), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rl_b(State* state){
	(*state->cpu->PC)++;
	rotl(state, (state->cpu->B), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rl_c(State* state){
	(*state->cpu->PC)++;
	rotl(state, (state->cpu->C), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rl_d(State* state){
	(*state->cpu->PC)++;
	rotl(state, (state->cpu->D), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rl_e(State* state){
	(*state->cpu->PC)++;
	rotl(state, (state->cpu->E), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rl_h(State* state){
	(*state->cpu->PC)++;
	rotl(state, (state->cpu->H), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rl_l(State* state){
	(*state->cpu->PC)++;
	rotl(state, (state->cpu->L), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rlhl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	rotl(state, &a, 1);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void rr_a(State* state){
	(*state->cpu->PC)++;
	rotr(state, (state->cpu->A), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rr_b(State* state){
	(*state->cpu->PC)++;
	rotr(state, (state->cpu->B), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rr_c(State* state){
	(*state->cpu->PC)++;
	rotr(state, (state->cpu->C), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rr_d(State* state){
	(*state->cpu->PC)++;
	rotr(state, (state->cpu->D), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rr_e(State* state){
	(*state->cpu->PC)++;
	rotr(state, (state->cpu->E), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rr_h(State* state){
	(*state->cpu->PC)++;
	rotr(state, (state->cpu->H), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rr_l(State* state){
	(*state->cpu->PC)++;
	rotr(state, (state->cpu->L), 1);
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void rrhl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	rotr(state, &a, 1);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void sla_a(State* state){
	(*state->cpu->PC)++;
	lshift(state, (state->cpu->A));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void sla_b(State* state){
	(*state->cpu->PC)++;
	lshift(state, (state->cpu->B));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void sla_c(State* state){
	(*state->cpu->PC)++;
	lshift(state, (state->cpu->C));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void sla_d(State* state){
	(*state->cpu->PC)++;
	lshift(state, (state->cpu->D));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void sla_e(State* state){
	(*state->cpu->PC)++;
	lshift(state, (state->cpu->E));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void sla_h(State* state){
	(*state->cpu->PC)++;
	lshift(state, (state->cpu->H));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void sla_l(State* state){
	(*state->cpu->PC)++;
	lshift(state, (state->cpu->L));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void slahl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	lshift(state, &a);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void sra_a(State* state){
	(*state->cpu->PC)++;
	rshift_retain_b7(state, (state->cpu->A));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void sra_b(State* state){
	(*state->cpu->PC)++;
	rshift_retain_b7(state, (state->cpu->B));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void sra_c(State* state){
	(*state->cpu->PC)++;
	rshift_retain_b7(state, (state->cpu->C));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void sra_d(State* state){
	(*state->cpu->PC)++;
	rshift_retain_b7(state, (state->cpu->D));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void sra_e(State* state){
	(*state->cpu->PC)++;
	rshift_retain_b7(state, (state->cpu->E));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void sra_h(State* state){
	(*state->cpu->PC)++;
	rshift_retain_b7(state, (state->cpu->H));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void sra_l(State* state){
	(*state->cpu->PC)++;
	rshift_retain_b7(state, (state->cpu->L));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void srahl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	rshift_retain_b7(state, &a);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void swap_a(State* state){
	(*state->cpu->PC)++;
	swp(state, (state->cpu->A));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void swap_b(State* state){
	(*state->cpu->PC)++;
	swp(state, (state->cpu->B));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void swap_c(State* state){
	(*state->cpu->PC)++;
	swp(state, (state->cpu->C));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void swap_d(State* state){
	(*state->cpu->PC)++;
	swp(state, (state->cpu->D));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void swap_e(State* state){
	(*state->cpu->PC)++;
	swp(state, (state->cpu->E));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void swap_h(State* state){
	(*state->cpu->PC)++;
	swp(state, (state->cpu->H));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void swap_l(State* state){
	(*state->cpu->PC)++;
	swp(state, (state->cpu->L));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void swaphl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	swp(state, &a);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void srl_a(State* state){
	(*state->cpu->PC)++;
	rshift(state, (state->cpu->A));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void srl_b(State* state){
	(*state->cpu->PC)++;
	rshift(state, (state->cpu->B));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void srl_c(State* state){
	(*state->cpu->PC)++;
	rshift(state, (state->cpu->C));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void srl_d(State* state){
	(*state->cpu->PC)++;
	rshift(state, (state->cpu->D));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void srl_e(State* state){
	(*state->cpu->PC)++;
	rshift(state, (state->cpu->E));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void srl_h(State* state){
	(*state->cpu->PC)++;
	rshift(state, (state->cpu->H));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void srl_l(State* state){
	(*state->cpu->PC)++;
	rshift(state, (state->cpu->L));
	(*state->cpu->PC)++;
	sleep_cycles(state, 2);
}

void srlhl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	rshift(state, &a);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_0_a(State* state){
	setZ(state, ~((*state->cpu->A) & (1 << 0)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_1_a(State* state){
	setZ(state, ~((*state->cpu->A) & (1 << 1)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_2_a(State* state){
	setZ(state, ~((*state->cpu->A) & (1 << 2)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_3_a(State* state){
	setZ(state, ~((*state->cpu->A) & (1 << 3)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_4_a(State* state){
	setZ(state, ~((*state->cpu->A) & (1 << 4)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_5_a(State* state){
	setZ(state, ~((*state->cpu->A) & (1 << 5)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_6_a(State* state){
	setZ(state, ~((*state->cpu->A) & (1 << 6)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_7_a(State* state){
	setZ(state, ~((*state->cpu->A) & (1 << 7)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_0_b(State* state){
	setZ(state, ~((*state->cpu->B) & (1 << 0)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_1_b(State* state){
	setZ(state, ~((*state->cpu->B) & (1 << 1)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_2_b(State* state){
	setZ(state, ~((*state->cpu->B) & (1 << 2)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_3_b(State* state){
	setZ(state, ~((*state->cpu->B) & (1 << 3)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_4_b(State* state){
	setZ(state, ~((*state->cpu->B) & (1 << 4)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_5_b(State* state){
	setZ(state, ~((*state->cpu->B) & (1 << 5)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_6_b(State* state){
	setZ(state, ~((*state->cpu->B) & (1 << 6)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_7_b(State* state){
	setZ(state, ~((*state->cpu->B) & (1 << 7)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_0_c(State* state){
	setZ(state, ~((*state->cpu->C) & (1 << 0)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_1_c(State* state){
	setZ(state, ~((*state->cpu->C) & (1 << 1)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_2_c(State* state){
	setZ(state, ~((*state->cpu->C) & (1 << 2)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_3_c(State* state){
	setZ(state, ~((*state->cpu->C) & (1 << 3)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_4_c(State* state){
	setZ(state, ~((*state->cpu->C) & (1 << 4)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_5_c(State* state){
	setZ(state, ~((*state->cpu->C) & (1 << 5)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_6_c(State* state){
	setZ(state, ~((*state->cpu->C) & (1 << 6)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_7_c(State* state){
	setZ(state, ~((*state->cpu->C) & (1 << 7)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_0_d(State* state){
	setZ(state, ~((*state->cpu->D) & (1 << 0)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_1_d(State* state){
	setZ(state, ~((*state->cpu->D) & (1 << 1)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_2_d(State* state){
	setZ(state, ~((*state->cpu->D) & (1 << 2)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_3_d(State* state){
	setZ(state, ~((*state->cpu->D) & (1 << 3)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_4_d(State* state){
	setZ(state, ~((*state->cpu->D) & (1 << 4)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_5_d(State* state){
	setZ(state, ~((*state->cpu->D) & (1 << 5)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_6_d(State* state){
	setZ(state, ~((*state->cpu->D) & (1 << 6)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_7_d(State* state){
	setZ(state, ~((*state->cpu->D) & (1 << 7)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_0_e(State* state){
	setZ(state, ~((*state->cpu->E) & (1 << 0)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_1_e(State* state){
	setZ(state, ~((*state->cpu->E) & (1 << 1)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_2_e(State* state){
	setZ(state, ~((*state->cpu->E) & (1 << 2)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_3_e(State* state){
	setZ(state, ~((*state->cpu->E) & (1 << 3)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_4_e(State* state){
	setZ(state, ~((*state->cpu->E) & (1 << 4)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_5_e(State* state){
	setZ(state, ~((*state->cpu->E) & (1 << 5)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_6_e(State* state){
	setZ(state, ~((*state->cpu->E) & (1 << 6)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_7_e(State* state){
	setZ(state, ~((*state->cpu->E) & (1 << 7)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_0_h(State* state){
	setZ(state, ~((*state->cpu->H) & (1 << 0)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_1_h(State* state){
	setZ(state, ~((*state->cpu->H) & (1 << 1)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_2_h(State* state){
	setZ(state, ~((*state->cpu->H) & (1 << 2)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_3_h(State* state){
	setZ(state, ~((*state->cpu->H) & (1 << 3)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_4_h(State* state){
	setZ(state, ~((*state->cpu->H) & (1 << 4)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_5_h(State* state){
	setZ(state, ~((*state->cpu->H) & (1 << 5)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_6_h(State* state){
	setZ(state, ~((*state->cpu->H) & (1 << 6)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_7_h(State* state){
	setZ(state, ~((*state->cpu->H) & (1 << 7)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_0_l(State* state){
	setZ(state, ~((*state->cpu->L) & (1 << 0)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_1_l(State* state){
	setZ(state, ~((*state->cpu->L) & (1 << 1)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_2_l(State* state){
	setZ(state, ~((*state->cpu->L) & (1 << 2)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_3_l(State* state){
	setZ(state, ~((*state->cpu->L) & (1 << 3)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_4_l(State* state){
	setZ(state, ~((*state->cpu->L) & (1 << 4)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_5_l(State* state){
	setZ(state, ~((*state->cpu->L) & (1 << 5)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_6_l(State* state){
	setZ(state, ~((*state->cpu->L) & (1 << 6)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_7_l(State* state){
	setZ(state, ~((*state->cpu->L) & (1 << 7)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_0_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	setZ(state, ~(a & (1 << 0)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_1_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	setZ(state, ~(a & (1 << 1)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_2_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	setZ(state, ~(a & (1 << 2)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_3_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	setZ(state, ~(a & (1 << 3)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_4_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	setZ(state, ~(a & (1 << 4)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_5_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	setZ(state, ~(a & (1 << 5)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_6_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	setZ(state, ~(a & (1 << 6)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void bit_7_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	setZ(state, ~(a & (1 << 7)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_0_a(State* state){
	*state->cpu->A &= (~(1 << 0));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_1_a(State* state){
	*state->cpu->A &= (~(1 << 1));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_2_a(State* state){
	*state->cpu->A &= (~(1 << 2));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_3_a(State* state){
	*state->cpu->A &= (~(1 << 3));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_4_a(State* state){
	*state->cpu->A &= (~(1 << 4));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_5_a(State* state){
	*state->cpu->A &= (~(1 << 5));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_6_a(State* state){
	*state->cpu->A &= (~(1 << 6));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_7_a(State* state){
	*state->cpu->A &= (~(1 << 7));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_0_b(State* state){
	*state->cpu->B &= (~(1 << 0));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_1_b(State* state){
	*state->cpu->B &= (~(1 << 1));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_2_b(State* state){
	*state->cpu->B &= (~(1 << 2));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_3_b(State* state){
	*state->cpu->B &= (~(1 << 3));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_4_b(State* state){
	*state->cpu->B &= (~(1 << 4));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_5_b(State* state){
	*state->cpu->B &= (~(1 << 5));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_6_b(State* state){
	*state->cpu->B &= (~(1 << 6));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_7_b(State* state){
	*state->cpu->B &= (~(1 << 7));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_0_c(State* state){
	*state->cpu->C &= (~(1 << 0));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_1_c(State* state){
	*state->cpu->C &= (~(1 << 1));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_2_c(State* state){
	*state->cpu->C &= (~(1 << 2));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_3_c(State* state){
	*state->cpu->C &= (~(1 << 3));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_4_c(State* state){
	*state->cpu->C &= (~(1 << 4));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_5_c(State* state){
	*state->cpu->C &= (~(1 << 5));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_6_c(State* state){
	*state->cpu->C &= (~(1 << 6));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_7_c(State* state){
	*state->cpu->C &= (~(1 << 7));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_0_d(State* state){
	*state->cpu->D &= (~(1 << 0));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_1_d(State* state){
	*state->cpu->D &= (~(1 << 1));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_2_d(State* state){
	*state->cpu->D &= (~(1 << 2));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_3_d(State* state){
	*state->cpu->D &= (~(1 << 3));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_4_d(State* state){
	*state->cpu->D &= (~(1 << 4));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_5_d(State* state){
	*state->cpu->D &= (~(1 << 5));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_6_d(State* state){
	*state->cpu->D &= (~(1 << 6));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_7_d(State* state){
	*state->cpu->D &= (~(1 << 7));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_0_e(State* state){
	*state->cpu->E &= (~(1 << 0));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_1_e(State* state){
	*state->cpu->E &= (~(1 << 1));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_2_e(State* state){
	*state->cpu->E &= (~(1 << 2));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_3_e(State* state){
	*state->cpu->E &= (~(1 << 3));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_4_e(State* state){
	*state->cpu->E &= (~(1 << 4));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_5_e(State* state){
	*state->cpu->E &= (~(1 << 5));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_6_e(State* state){
	*state->cpu->E &= (~(1 << 6));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_7_e(State* state){
	*state->cpu->E &= (~(1 << 7));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_0_h(State* state){
	*state->cpu->H &= (~(1 << 0));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_1_h(State* state){
	*state->cpu->H &= (~(1 << 1));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_2_h(State* state){
	*state->cpu->H &= (~(1 << 2));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_3_h(State* state){
	*state->cpu->H &= (~(1 << 3));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_4_h(State* state){
	*state->cpu->H &= (~(1 << 4));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_5_h(State* state){
	*state->cpu->H &= (~(1 << 5));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_6_h(State* state){
	*state->cpu->H &= (~(1 << 6));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_7_h(State* state){
	*state->cpu->H &= (~(1 << 7));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_0_l(State* state){
	*state->cpu->L &= (~(1 << 0));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_1_l(State* state){
	*state->cpu->L &= (~(1 << 1));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_2_l(State* state){
	*state->cpu->L &= (~(1 << 2));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_3_l(State* state){
	*state->cpu->L &= (~(1 << 3));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_4_l(State* state){
	*state->cpu->L &= (~(1 << 4));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_5_l(State* state){
	*state->cpu->L &= (~(1 << 5));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_6_l(State* state){
	*state->cpu->L &= (~(1 << 6));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_7_l(State* state){
	*state->cpu->L &= (~(1 << 7));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_0_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	mm_write(state, *state->cpu->HL, a & (~(1 << 0)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_1_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	mm_write(state, *state->cpu->HL, a & (~(1 << 1)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_2_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	mm_write(state, *state->cpu->HL, a & (~(1 << 2)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_3_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	mm_write(state, *state->cpu->HL, a & (~(1 << 3)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_4_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	mm_write(state, *state->cpu->HL, a & (~(1 << 4)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_5_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	mm_write(state, *state->cpu->HL, a & (~(1 << 5)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_6_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	mm_write(state, *state->cpu->HL, a & (~(1 << 6)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void res_7_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	mm_write(state, *state->cpu->HL, a & (~(1 << 7)));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_0_a(State* state){
	*state->cpu->A |= (1 << 0);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_1_a(State* state){
	*state->cpu->A |= (1 << 1);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_2_a(State* state){
	*state->cpu->A |= (1 << 2);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_3_a(State* state){
	*state->cpu->A |= (1 << 3);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_4_a(State* state){
	*state->cpu->A |= (1 << 4);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_5_a(State* state){
	*state->cpu->A |= (1 << 5);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_6_a(State* state){
	*state->cpu->A |= (1 << 6);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_7_a(State* state){
	*state->cpu->A |= (1 << 7);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_0_b(State* state){
	*state->cpu->B |= (1 << 0);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_1_b(State* state){
	*state->cpu->B |= (1 << 1);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_2_b(State* state){
	*state->cpu->B |= (1 << 2);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_3_b(State* state){
	*state->cpu->B |= (1 << 3);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_4_b(State* state){
	*state->cpu->B |= (1 << 4);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_5_b(State* state){
	*state->cpu->B |= (1 << 5);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_6_b(State* state){
	*state->cpu->B |= (1 << 6);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_7_b(State* state){
	*state->cpu->B |= (1 << 7);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_0_c(State* state){
	*state->cpu->C |= (1 << 0);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_1_c(State* state){
	*state->cpu->C |= (1 << 1);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_2_c(State* state){
	*state->cpu->C |= (1 << 2);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_3_c(State* state){
	*state->cpu->C |= (1 << 3);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_4_c(State* state){
	*state->cpu->C |= (1 << 4);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_5_c(State* state){
	*state->cpu->C |= (1 << 5);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_6_c(State* state){
	*state->cpu->C |= (1 << 6);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_7_c(State* state){
	*state->cpu->C |= (1 << 7);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_0_d(State* state){
	*state->cpu->D |= (1 << 0);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_1_d(State* state){
	*state->cpu->D |= (1 << 1);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_2_d(State* state){
	*state->cpu->D |= (1 << 2);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_3_d(State* state){
	*state->cpu->D |= (1 << 3);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_4_d(State* state){
	*state->cpu->D |= (1 << 4);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_5_d(State* state){
	*state->cpu->D |= (1 << 5);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_6_d(State* state){
	*state->cpu->D |= (1 << 6);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_7_d(State* state){
	*state->cpu->D |= (1 << 7);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_0_e(State* state){
	*state->cpu->E |= (1 << 0);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_1_e(State* state){
	*state->cpu->E |= (1 << 1);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_2_e(State* state){
	*state->cpu->E |= (1 << 2);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_3_e(State* state){
	*state->cpu->E |= (1 << 3);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_4_e(State* state){
	*state->cpu->E |= (1 << 4);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_5_e(State* state){
	*state->cpu->E |= (1 << 5);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_6_e(State* state){
	*state->cpu->E |= (1 << 6);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_7_e(State* state){
	*state->cpu->E |= (1 << 7);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_0_h(State* state){
	*state->cpu->H |= (1 << 0);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_1_h(State* state){
	*state->cpu->H |= (1 << 1);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_2_h(State* state){
	*state->cpu->H |= (1 << 2);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_3_h(State* state){
	*state->cpu->H |= (1 << 3);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_4_h(State* state){
	*state->cpu->H |= (1 << 4);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_5_h(State* state){
	*state->cpu->H |= (1 << 5);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_6_h(State* state){
	*state->cpu->H |= (1 << 6);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_7_h(State* state){
	*state->cpu->H |= (1 << 7);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_0_l(State* state){
	*state->cpu->L |= (1 << 0);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_1_l(State* state){
	*state->cpu->L |= (1 << 1);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_2_l(State* state){
	*state->cpu->L |= (1 << 2);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_3_l(State* state){
	*state->cpu->L |= (1 << 3);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_4_l(State* state){
	*state->cpu->L |= (1 << 4);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_5_l(State* state){
	*state->cpu->L |= (1 << 5);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_6_l(State* state){
	*state->cpu->L |= (1 << 6);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_7_l(State* state){
	*state->cpu->L |= (1 << 7);
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_0_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	mm_write(state, *state->cpu->HL, a | (1 << 0));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_1_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	mm_write(state, *state->cpu->HL, a | (1 << 1));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_2_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	mm_write(state, *state->cpu->HL, a | (1 << 2));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_3_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	mm_write(state, *state->cpu->HL, a | (1 << 3));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_4_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	mm_write(state, *state->cpu->HL, a | (1 << 4));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_5_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	mm_write(state, *state->cpu->HL, a | (1 << 5));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_6_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	mm_write(state, *state->cpu->HL, a | (1 << 6));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

void set_7_hl(State* state){
	uint8_t a = mm_read(state, *state->cpu->HL);
	mm_write(state, *state->cpu->HL, a | (1 << 7));
	(*state->cpu->PC) += 2;
	sleep_cycles(state, 2);
}

InstructionFn prefixed_instruction_map[0x100] = {
    rlc_b, rlc_c, rlc_d, rlc_e, rlc_h, rlc_l, rlchl, rlc_a, rrc_b, rrc_c, rrc_d, rrc_e, rrc_h, rrc_l, rrchl, rrc_a,
    rl_b, rl_c, rl_d, rl_e, rl_h, rl_l, rlhl, rl_a, rr_b, rr_c, rr_d, rr_e, rr_h, rr_l, rrhl, rr_a,
    sla_b, sla_c, sla_d, sla_e, sla_h, sla_l, slahl, sla_a, sra_b, sra_c, sra_d, sra_e, sra_h, sra_l, srahl, sra_a,
    swap_b, swap_c, swap_d, swap_e, swap_h, swap_l, swaphl, swap_a, srl_b, srl_c, srl_d, srl_e, srl_h, srl_l, srlhl, srl_a,
    bit_0_b, bit_0_c, bit_0_d, bit_0_e, bit_0_h, bit_0_l, bit_0_hl, bit_0_a, bit_1_b, bit_1_c, bit_1_d, bit_1_e, bit_1_h, bit_1_l, bit_1_hl, bit_1_a,
    bit_2_b, bit_2_c, bit_2_d, bit_2_e, bit_2_h, bit_2_l, bit_2_hl, bit_2_a, bit_3_b, bit_3_c, bit_3_d, bit_3_e, bit_3_h, bit_3_l, bit_3_hl, bit_3_a,
    bit_4_b, bit_4_c, bit_4_d, bit_4_e, bit_4_h, bit_4_l, bit_4_hl, bit_4_a, bit_5_b, bit_5_c, bit_5_d, bit_5_e, bit_5_h, bit_5_l, bit_5_hl, bit_5_a,
    bit_6_b, bit_6_c, bit_6_d, bit_6_e, bit_6_h, bit_6_l, bit_6_hl, bit_6_a, bit_7_b, bit_7_c, bit_7_d, bit_7_e, bit_7_h, bit_7_l, bit_7_hl, bit_7_a,
    res_0_b, res_0_c, res_0_d, res_0_e, res_0_h, res_0_l, res_0_hl, res_0_a, res_1_b, res_1_c, res_1_d, res_1_e, res_1_h, res_1_l, res_1_hl, res_1_a,
    res_2_b, res_2_c, res_2_d, res_2_e, res_2_h, res_2_l, res_2_hl, res_2_a, res_3_b, res_3_c, res_3_d, res_3_e, res_3_h, res_3_l, res_3_hl, res_3_a,
    res_4_b, res_4_c, res_4_d, res_4_e, res_4_h, res_4_l, res_4_hl, res_4_a, res_5_b, res_5_c, res_5_d, res_5_e, res_5_h, res_5_l, res_5_hl, res_5_a,
    res_6_b, res_6_c, res_6_d, res_6_e, res_6_h, res_6_l, res_6_hl, res_6_a, res_7_b, res_7_c, res_7_d, res_7_e, res_7_h, res_7_l, res_7_hl, res_7_a,
    set_0_b, set_0_c, set_0_d, set_0_e, set_0_h, set_0_l, set_0_hl, set_0_a, set_1_b, set_1_c, set_1_d, set_1_e, set_1_h, set_1_l, set_1_hl, set_1_a,
    set_2_b, set_2_c, set_2_d, set_2_e, set_2_h, set_2_l, set_2_hl, set_2_a, set_3_b, set_3_c, set_3_d, set_3_e, set_3_h, set_3_l, set_3_hl, set_3_a,
    set_4_b, set_4_c, set_4_d, set_4_e, set_4_h, set_4_l, set_4_hl, set_4_a, set_5_b, set_5_c, set_5_d, set_5_e, set_5_h, set_5_l, set_5_hl, set_5_a,
    set_6_b, set_6_c, set_6_d, set_6_e, set_6_h, set_6_l, set_6_hl, set_6_a, set_7_b, set_7_c, set_7_d, set_7_e, set_7_h, set_7_l, set_7_hl, set_7_a
};
