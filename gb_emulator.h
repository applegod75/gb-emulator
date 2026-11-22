#ifndef GB_EMULATOR_GB_EMULATOR_H_
#define GB_EMULATOR_GB_EMULATOR_H_

#include "memory_map.h"
#include "state.h"
#include "argparse.h"

#define WIDTH  160
#define HEIGHT 144

State* initGameboy(Args* a);
void freeGameboy(State* state);

#endif // GB_EMULATOR_GB_EMULATOR_H_
