#ifndef GB_EMULATOR_CPU_H_
#define GB_EMULATOR_CPU_H_

#include <stdint.h>
#include <stdlib.h>
#include "state.h"

CpuState* cpu_init();
void cpu_cycle(State* state);

#endif // GB_EMULATOR_CPU_H_
