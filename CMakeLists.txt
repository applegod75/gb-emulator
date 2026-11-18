cmake_minimum_required(VERSION 3.17.0)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED true)

project(gb_emulator C)

add_executable(gb_emulator)
add_subdirectory(SDL)

dep_option(SDL_TESTS "build sdl tests" OFF SDL_TEST OFF)

target_sources(gb_emulator PRIVATE
    main.c
    memory_map.c
    cartridge.c
    cpu.c
    argparse.c
    gb_emulator.c
)

target_link_libraries(gb_emulator PRIVATE SDL3::SDL3)
