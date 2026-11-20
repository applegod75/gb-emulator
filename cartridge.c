#include "cartridge.h"
#include <stdio.h>
#include <stdlib.h>

uint64_t getFileSize(FILE* f){
    fseek(f, 0, SEEK_END);
    uint64_t pos = ftell(f);
    fseek(f, 0, SEEK_SET);
    return pos;
}

uint8_t isCartridgeType(uint8_t v) {
    return ( v == ROM_ONLY                          ||
             v == MBC1                              ||
             v == MBC1_RAM                          ||
             v == MBC1_RAM_BATTERY                  ||
             v == MBC2                              ||
             v == MBC2_BATTERY                      ||
             v == ROM_RAM                           ||
             v == ROM_RAM_BATTERY                   ||
             v == MMM01                             ||
             v == MMM01_RAM                         ||
             v == MMM01_RAM_BATTERY                 ||
             v == MBC3_TIMER_BATTERY                ||
             v == MBC3_TIMER_RAM_BATTERY            ||
             v == MBC3                              ||
             v == MBC3_RAM                          ||
             v == MBC3_RAM_BATTERY                  ||
             v == MBC5                              ||
             v == MBC5_RAM                          ||
             v == MBC5_RAM_BATTERY                  ||
             v == MBC5_RUMBLE                       ||
             v == MBC5_RUMBLE_RAM                   ||
             v == MBC5_RUMBLE_RAM_BATTERY           ||
             v == MBC6                              ||
             v == MBC7_SENSOR_RUMBLE_RAM_BATTERY    ||
             v == POCKET_CAMERA                     ||
             v == BANDAI_THAMA5                     ||
             v == HUC3                              ||
             v == HUC1_RAM_BATTERY
    );
}

uint8_t cartridge_hasExternalRam(CartridgeType v) {
    return ( v == MBC1_RAM                          ||
             v == MBC1_RAM_BATTERY                  ||
             v == ROM_RAM                           ||
             v == ROM_RAM_BATTERY                   ||
             v == MMM01_RAM                         ||
             v == MMM01_RAM_BATTERY                 ||
             v == MBC3_TIMER_RAM_BATTERY            ||
             v == MBC3_RAM                          ||
             v == MBC3_RAM_BATTERY                  ||
             v == MBC3_TIMER_RAM_BATTERY            ||
             v == MBC5_RAM                          ||
             v == MBC5_RAM_BATTERY                  ||
             v == MBC5_RUMBLE_RAM                   ||
             v == MBC5_RUMBLE_RAM_BATTERY           ||
             v == MBC7_SENSOR_RUMBLE_RAM_BATTERY    ||
             v == HUC1_RAM_BATTERY
    );
}

Cartridge* cartridge_load(char* file) {
    FILE* f = fopen(file, "rb");
    if (f == NULL) {
        printf("failed to open cartridge file\n");
        return NULL;
    }
    uint64_t file_size = getFileSize(f);

    // the most the game boy color could handle was an 8MiB rom
    if (file_size > (uint64_t)1024 * 1024 * 1024 * 8) {
        printf("rom is too big\n");
        return NULL;
    } else if (file_size < (uint64_t)16 * 1024) { // minimum rom size
        printf("rom is too small\n");
        return NULL;
    }

    unsigned char* buf = (unsigned char*)malloc(file_size);
    fread(buf, sizeof(unsigned char), file_size, f);
    fclose(f);

    Cartridge* rtn = (Cartridge*)malloc(sizeof(Cartridge));
    rtn->data = (void*)buf;
    rtn->cgb_flag = ((uint8_t*)rtn->data)[0x0143];
    rtn->sgb_flag = ((uint8_t*)rtn->data)[0x0146];
    
    uint8_t rom_header_size = ((uint8_t*)rtn->data)[0x0148];
    uint64_t rom_actual_size = 0;

    if (rom_header_size <= 0x08) {
        rom_actual_size = (32 * 1024) << rom_header_size;
    } else {
        switch (rom_header_size) {
            case 0x52: rom_actual_size = (uint64_t)(16 * 1024 * 72); break;
            case 0x53: rom_actual_size = (uint64_t)(16 * 1024 * 80); break;
            case 0x54: rom_actual_size = (uint64_t)(16 * 1024 * 96); break;
            default: rom_actual_size = 0;
        }
    }
    
    if (rom_actual_size == 0) {
        printf("rom size header corrupted\n");
        return NULL;
    }
    rtn->rom_size = rom_actual_size;
    
    uint8_t ram_header_size = ((uint8_t*)rtn->data)[0x0149];
    uint64_t ram_actual_size = 0;
    switch (ram_header_size) {
        case 0x00: ram_actual_size = 0;           break;
        case 0x01: printf("ram size header corrupted\n"); return NULL;
        case 0x02: ram_actual_size =   8 * 1024;  break;
        case 0x03: ram_actual_size =  32 * 1024;  break;
        case 0x04: ram_actual_size = 128 * 1024;  break;
        case 0x05: ram_actual_size =  64 * 1024;  break;
        default: printf("ram size header corrupted\n"); return NULL;
    }

    rtn->ram_size = ram_actual_size;
    uint8_t type = ((uint8_t*)rtn->data)[0x0147];
    if (!isCartridgeType(type)){
        printf("cartridge type header was corrupted\n");
        return NULL;
    }

    rtn->type = (CartridgeType)type;

    return rtn;
}
