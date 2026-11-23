#include "gb_emulator.h"
#include <stdlib.h>
#include "cpu.h"
#include <stdio.h>

uint64_t filesize(FILE* f){
    fseek(f, 0, SEEK_END);
    uint64_t rtn = ftell(f);
    fseek(f, 0, SEEK_SET);
    return rtn;
}

State* initGameboy(Args* a){
    State* rtn = (State*)malloc(sizeof(State));
    rtn->current_cycle = 0;
    rtn->cpu = cpu_init();
    rtn->loaded_cartridge = cartridge_load(a->rom_name);
    
    if (rtn->loaded_cartridge == NULL){
        printf("failed to load cartridge\n");
        return NULL;
    }

    rtn->mem = mm_create(rtn->loaded_cartridge);
    
    if (a->skip_bootrom) {
        *rtn->cpu->AF = (0x0001 << 8) | 0xB0;
        *rtn->cpu->B  =  0x0000;
        *rtn->cpu->C  =  0x0013;
        *rtn->cpu->D  =  0x0000;
        *rtn->cpu->E  =  0x00D8;
        *rtn->cpu->H  =  0x0001;
        *rtn->cpu->L  =  0x004D;
        *rtn->cpu->SP =  0xFFFE;
        *rtn->cpu->PC =  0x0100;
    } else {
        FILE* bootrom_file = fopen(a->bootrom_name, "rb");
        
        if (bootrom_file == NULL) {
            printf("could not open bootrom file\n");
            return NULL;
        }

        uint64_t file_size = filesize(bootrom_file);
        if (file_size != 0x100) {
            printf("bootrom is of unexpected size 0x%llX", file_size);
        }

        void* data = malloc(0x100);

        fread(data, 1, 0x100, bootrom_file);
        
        memcpy(rtn->mem->rom_bank_0, data, 0x100);
    }
    
    rtn->pixel_buf = (Uint32*)malloc(WIDTH * HEIGHT * sizeof(Uint32));
    memset(rtn->pixel_buf, 0, WIDTH * HEIGHT * sizeof(Uint32));

    return rtn;
}

void freeGameboy(State* state) {
    cartridge_free(state->loaded_cartridge);
    free(state->pixel_buf);
    cpu_free(state->cpu);
    mm_free(state->mem);
    free(state);
}
