#include "memory_map.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define ROM_BANK_SIZE (16 * 1024)

void dispatchRomWrite(State* state, uint16_t address, uint8_t val){
    if (address > 0x7FFF) return; // not a rom write
}

void dispatchIoWrite(State* state, uint16_t address, uint8_t val){
    if (address > 0xFF70 || address < 0xFF00) return;
    
    if (address == 0xFF50 && val != 0) { // unmap bootrom
        memcpy(state->mem->rom_bank_0, state->loaded_cartridge->data, 0xFF);
    }
}

uint8_t mm_read(State* state, uint16_t address){
    if (address <= 0x3FFF) return state->mem->rom_bank_0         [address];
    if (address <= 0x7FFF) return state->mem->selected_rom_bank  [(address - 0x4000)];
    if (address <= 0x9FFF) return state->mem->selected_vram      [(address - 0x8000)];
    if (address <= 0xBFFF) return state->mem->external_ram       [(address - 0xA000)];
    if (address <= 0xCFFF) return state->mem->wram               [(address - 0xC000)];
    if (address <= 0xDFFF) return state->mem->selected_wram      [(address - 0xD000)];
    if (address <= 0xFDFF) return mm_read(state, address - 0x2000);
    if (address <= 0xFE9F) return state->mem->OAM                [(address - 0xFE00)];
    /// TODO: the below return value actually depends on cpu state, add a check for that.
    if (address <= 0xFEFF) return 0x00;
    /// TODO: implement I/O mappings and use them here.
    if (address <= 0xFF7F) return 0x00;
    if (address <= 0xFFFE) return state->mem->hram               [(address - 0xFF80)];
    if (address <= 0xFFFF) return *state->mem->IE;
}

void mm_write(State* state, uint16_t address, uint8_t val){
    // here else if's are actually required because the fn doesn't return after every if statement.
    if (address <= 0x7FFF) dispatchRomWrite(state, address, val);
    else if (address <= 0x9FFF) state->mem->selected_vram        [(address - 0x8000)] = val;
    else if (address <= 0xBFFF) state->mem->external_ram         [(address - 0xA000)] = val;
    else if (address <= 0xCFFF) state->mem->wram                 [(address - 0xC000)] = val;
    else if (address <= 0xDFFF) state->mem->selected_wram        [(address - 0xD000)] = val;
    else if (address <= 0xFDFF) mm_write(state, address - 0x2000, val);
    else if (address <= 0xFE9F) state->mem->OAM                  [(address - 0xFE00)] = val;
    else if (address <= 0xFEFF) return;
    /// TODO: implement i/o mappings here
    else if (address <= 0xFF7F) dispatchIoWrite(state, address, val);
    else if (address <= 0xFFFE) state->mem->hram                 [(address - 0xFF80)] = val;
    else if (address <= 0xFFFF && val == 1) *state->mem->IE = 1;
}

MappedMemory* mm_create(Cartridge* cartridge){ 
    MappedMemory* rtn = (MappedMemory*)malloc(sizeof(MappedMemory));
    rtn->rom_bank_0 = (uint8_t*)malloc(ROM_BANK_SIZE);
    memcpy(rtn->rom_bank_0, cartridge->data, ROM_BANK_SIZE);
    
    rtn->selected_rom_bank = (uint8_t*)malloc(ROM_BANK_SIZE);

    if (cartridge->has_rombank_01) {
        memcpy(rtn->selected_rom_bank, (uint8_t*)cartridge->data + ROM_BANK_SIZE, ROM_BANK_SIZE);
    } else {
        memset(rtn->selected_rom_bank, 0, ROM_BANK_SIZE);
    }

    rtn->selected_vram = (uint8_t*)malloc(8 * 1024);
    memset(rtn->selected_vram, 0, 8 * 1024);

    if (cartridge_hasExternalRam(cartridge->type)) {
        rtn->external_ram = (uint8_t*)malloc(8 * 1024);
        memset(rtn->external_ram, 0, 8 * 1024);
    } else {
        rtn->external_ram = NULL;
    }
   
    uint8_t* ram_memory_pool = (uint8_t*)malloc(8 * 1024);
    memset(ram_memory_pool, 0, 8 * 1024);

    rtn->wram = ram_memory_pool;
    rtn->selected_wram = ram_memory_pool + (4 * 1024);

    rtn->OAM =  (uint8_t*)malloc(0x9F);
    memset(rtn->OAM , 0, 0x9F);
    rtn->hram = (uint8_t*)malloc(0x7E);
    memset(rtn->hram, 0, 0x7E);

    rtn->IE = (uint8_t*)malloc(1);
    *rtn->IE = 0;
    return rtn;
}

void mm_free(MappedMemory* mm){
    free(mm->rom_bank_0);
    free(mm->selected_rom_bank);
    free(mm->selected_vram);
    if (mm->external_ram != NULL) free(mm->external_ram);
    // selected_wram and wram are in contiguous memory.
    free(mm->wram);
    free(mm->OAM);
    free(mm->hram);
    free(mm->IE);
}

