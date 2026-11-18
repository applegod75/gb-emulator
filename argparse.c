#include "argparse.h"
#include <stdio.h>
#include <stdlib.h>

uint8_t fileExists(char* name){
    FILE* f = fopen(name, "r");
    if (f == NULL) return 0;
    fclose(f);
    return 1;
}

Args* parseArgs(int argc, char** argv){
    if (argc < 2) {
        printf("usage: gb_emulator [OPTIONS] [ROM]");
        return NULL; 
    }

    Args* rtn = (Args*)malloc(sizeof(Args));
    
    if (!fileExists(argv[argc - 1])) {
        printf("Could not find selected rom");
        return NULL;
    }

    rtn->rom_name = argv[argc - 1];
    if (argc == 2) return rtn;

    // skip over the exe and rom name
    for (uint32_t i = 1; i < (argc - 1); i++) {
        if (strcmp("--disable-bootrom", argv[i]) == 0) {
            rtn->skip_bootrom = 1;
        } else {
            printf("skipping unknown flag '%s'", argv[i]);
        }
    }

    return rtn;
}
