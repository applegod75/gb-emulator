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
    rtn->bootrom_name = NULL;
    rtn->skip_bootrom = 0;

    // skip over the exe and rom name
    for (uint32_t i = 1; i < (argc - 1); i++) {
        if (strcmp("--disable-bootrom", argv[i]) == 0) {
            rtn->skip_bootrom = 1;
        } else if (strcmp("--bootrom", argv[i]) == 0) {
            if (argc - 1 == i + 1) {
                printf("passed --bootrom without filename, ignoring\n");
            } else {
                rtn->bootrom_name = argv[i + 1];
                i++;
            }
        } else if (strcmp("--debug-mode", argv[i]) == 0) {
            printf("WARN: debug mode permanently changes the behaviour of the STOP instruction, and should never be used if you are not actively ");
            printf("debugging the gameboy emulator itself.\n");
            rtn->debug_mode = 1;
        } else {
            printf("skipping unknown flag '%s'", argv[i]);
        } 
    }

    if (rtn->bootrom_name == NULL) {
        printf("bootrom name was not specified, skipping bootrom\n");
        rtn->skip_bootrom = 1;
    }

    return rtn;
}
