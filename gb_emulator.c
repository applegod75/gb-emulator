#include "gb_emulator.h"

State* initGameboy(Args* a){
    State* rtn = (State*)malloc(sizeof(State));
    rtn->cpu = cpu_init();
    rtn->loaded_cartridge = cartridge_load(a->rom_name);
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
        memcpy(rtn->mem->rom_bank_0, rtn->loaded_cartridge->data, 0xFF);
    }

    return rtn;
}
