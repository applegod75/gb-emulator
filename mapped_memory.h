#ifndef GB_EMULATOR_MAPPED_MEMORY_H_
#define GB_EMULATOR_MAPPED_MEMORY_H_

typedef struct {
    uint8_t* rom_bank_0;
    uint8_t* selected_rom_bank;
    uint8_t* selected_vram;
    uint8_t* external_ram;
    uint8_t* wram;
    uint8_t* selected_wram;
    uint8_t* OAM;
    uint8_t* hram;
    uint8_t* IE;
} MappedMemory;


#endif // GB_EMULATOR_MAPPED_MEMORY_H_
