#ifndef GB_EMULATOR_STATE_H_
#define GB_EMULATOR_STATE_H_

#include <SDL3/SDL.h>

#include "cartridge.h"
#include "cpu_state.h"
#include "mapped_memory.h"

typedef struct {
    Cartridge* loaded_cartridge;
    CpuState* cpu;
    MappedMemory* mem;
    uint64_t current_cycle;
    Uint64 start_time;
    Uint32* pixel_buf;
    uint8_t debug_mode;
    uint8_t should_enable_interrupts;
} State;


#endif
