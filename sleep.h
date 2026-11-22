#ifndef GB_EMULATOR_SLEEP_H_
#define GB_EMULATOR_SLEEP_H_

#define CPU_CLOCK_TIME (1.0f / 1048576)

#include "state.h"

void sleep_cycles(State* state, uint8_t amount);
void sleep_cycle(State* state);

#endif
