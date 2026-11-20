#ifndef GB_EMULATOR_STATE_H_
#define GB_EMULATOR_STATE_H_

#include "cartridge.h"
#include "cpu_state.h"
#include "mapped_memory.h"

typedef struct {
    Cartridge* loaded_cartridge;
    CpuState* cpu;
    MappedMemory* mem;
} State;


#endif
