
#ifndef GB_EMULATOR_MEMORY_MAP_H_
#define GB_EMULATOR_MEMORY_MAP_H_

#include <stdint.h>
#include "cartridge.h"
#include "state.h"
#include "mapped_memory.h"

MappedMemory* mm_create(Cartridge* cartridge);
uint8_t mm_read(State* state, uint16_t address);
void mm_write(State* state, uint16_t address, uint8_t val);

#endif // GB_EMULATOR_MEMORY_MAP_H_
