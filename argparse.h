#ifndef GB_EMULATOR_ARGPARSE_H_
#define GB_EMULATOR_ARGPARSE_H_

#include <string.h>
#include <stdint.h>

typedef struct {
    uint8_t skip_bootrom;
    char* rom_name;
    char* bootrom_name;
} Args;

Args* parseArgs(int argc, char** argv);


#endif
