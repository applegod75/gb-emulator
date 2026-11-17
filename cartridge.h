#ifndef GB_EMULATOR_CARTRIDGE_H_
#define GB_EMULATOR_CARTRIDGE_H_

#include <stdint.h>
typedef enum {
    ROM_ONLY =                          0x00,
    MBC1 =                              0x01,
    MBC1_RAM =                          0x02,
    MBC1_RAM_BATTERY =                  0x03,
    MBC2 =                              0x05,
    MBC2_BATTERY =                      0x06,
    ROM_RAM =                           0x08,
    ROM_RAM_BATTERY =                   0x09,
    MMM01 =                             0x0B,
    MMM01_RAM =                         0x0C,
    MMM01_RAM_BATTERY =                 0x0D,
    MBC3_TIMER_BATTERY =                0x0F,
    MBC3_TIMER_RAM_BATTERY =            0x10,
    MBC3 =                              0x11,
    MBC3_RAM =                          0x12,
    MBC3_RAM_BATTERY =                  0x13,
    MBC5 =                              0x19,
    MBC5_RAM =                          0x1A,
    MBC5_RAM_BATTERY =                  0x1B,
    MBC5_RUMBLE =                       0x1C,
    MBC5_RUMBLE_RAM =                   0x1D,
    MBC5_RUMBLE_RAM_BATTERY =           0x1E,
    MBC6 =                              0x20,
    MBC7_SENSOR_RUMBLE_RAM_BATTERY =    0x22,
    POCKET_CAMERA =                     0xFC,
    BANDAI_THAMA5 =                     0xFD,
    HUC3 =                              0xFE,
    HUC1_RAM_BATTERY =                  0xFF
} CartridgeType;

typedef struct {
    void* data;
    uint8_t cgb_flag;
    uint8_t sgb_flag;
    CartridgeType type;
    uint8_t rom_size;
    uint8_t ram_size;
} Cartridge;

uint8_t cartridge_hasExternalRam(CartridgeType type);

Cartridge* cartridge_load(char* file);

#endif // GB_EMULATOR_CARTRIDGE_H_
