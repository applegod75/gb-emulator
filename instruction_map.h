#ifndef GB_EMULATOR_INSTRUCTION_MAP_H_
#define GB_EMULATOR_INSTRUCTION_MAP_H_

#include "state.h"

typedef void(*InstructionFn)(State*);
extern InstructionFn instruction_map[0x2C];
extern InstructionFn prefixed_instruction_map[0x1];

#endif
