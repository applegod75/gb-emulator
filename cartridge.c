#include "cartridge.h"
#include <stdio.h>
#include <stdlib.h>

// this is very beautiful, I'm aware.
void* bootrom = (void*)(
    "\x31\xFE\xFF\xAF\x21\xFF\x9F\x32\xCB\x7C\x20\xFB\x21\x26\xFF\x0E"
    "\x11\x3E\x80\x32\xE2\x0C\x3E\xF3\xE2\x32\x3E\x77\x77\x3E\xFC\xE0"
    "\x47\x11\x04\x01\x21\x10\x80\x1A\xCD\x95\x00\xCD\x96\x00\x13\x7B"
    "\xFE\x34\x20\xF3\x11\xD8\x00\x06\x08\x1A\x13\x22\x23\x05\x20\xF9"
    "\x3E\x19\xEA\x10\x99\x21\x2F\x99\x0E\x0C\x3D\x28\x08\x32\x0D\x20"
    "\xF9\x2E\x0F\x18\xF3\x67\x3E\x64\x57\xE0\x42\x3E\x91\xE0\x40\x04"
    "\x1E\x02\x0E\x0C\xF0\x44\xFE\x90\x20\xFA\x0D\x20\xF7\x1D\x20\xF2"
    "\x0E\x13\x24\x7C\x1E\x83\xFE\x62\x28\x06\x1E\xC1\xFE\x64\x20\x06"
    "\x7B\xE2\x0C\x3E\x87\xE2\xF0\x42\x90\xE0\x42\x15\x20\xD2\x05\x20"
    "\x4F\x16\x20\x18\xCB\x4F\x06\x04\xC5\xCB\x11\x17\xC1\xCB\x11\x17"
    "\x05\x20\xF5\x22\x23\x22\x23\xC9\xCE\xED\x66\x66\xCC\x0D\x00\x0B"
    "\x03\x73\x00\x83\x00\x0C\x00\x0D\x00\x08\x11\x1F\x88\x89\x00\x0E"
    "\xDC\xCC\x6E\xE6\xDD\xDD\xD9\x99\xBB\xBB\x67\x63\x6E\x0E\xEC\xCC"
    "\xDD\xDC\x99\x9F\xBB\xB9\x33\x3E\x3C\x42\xB9\xA5\xB9\xA5\x42\x3C"
    "\x21\x04\x01\x11\xA8\x00\x1A\x13\xBE\x20\xFE\x23\x7D\xFE\x34\x20"
    "\xF5\x06\x19\x78\x86\x23\x05\x20\xFB\x86\x20\xFE\x3E\x01\xE0\x50"
);

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
    if (f == NULL) return NULL;
    uint64_t file_size = getFileSize(f);

    // the most the game boy color could handle was an 8MiB rom
    if (file_size > (uint64_t)1024 * 1024 * 1024 * 8) {
        return NULL;
    } else if (file_size < (uint64_t)32 * 1024) { // minimum rom size
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
            default: return NULL;
        }
    }
    
    if (rom_actual_size == 0) return NULL;
    rtn->rom_size = rom_actual_size;
    
    uint8_t ram_header_size = ((uint8_t*)rtn->data)[0x0149];
    uint64_t ram_actual_size = 0;
    switch (ram_header_size) {
        case 0x00: ram_actual_size = 0;           break;
        case 0x01: return NULL;
        case 0x02: ram_actual_size =   8 * 1024;  break;
        case 0x03: ram_actual_size =  32 * 1024;  break;
        case 0x04: ram_actual_size = 128 * 1024;  break;
        case 0x05: ram_actual_size =  64 * 1024;  break;
        default: return NULL;
    }

    rtn->ram_size = ram_actual_size;
    uint8_t type = ((uint8_t*)rtn->data)[0x0147];
    if (!isCartridgeType(type)){
        return NULL;
    }

    rtn->type = (CartridgeType)type;

    return rtn;
}
