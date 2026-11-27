#ifndef GB_EMULATOR_CPU_STATE_H_
#define GB_EMULATOR_CPU_STATE_H_

#include <stdint.h>

typedef struct {
    uint16_t* AF; uint8_t* A;
    uint16_t* BC; uint8_t* B; uint8_t* C;
    uint16_t* DE; uint8_t* D; uint8_t* E;
    uint16_t* HL; uint8_t* H; uint8_t* L;
    uint16_t* SP;
    uint16_t* PC;
    uint8_t IME;
} CpuState;

#endif
