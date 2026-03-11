#include <cstdint>
#pragma once

class Chip8
{
public:
    uint8_t registers[16]{}; // 16 8-bit registers
    uint8_t mem[4096]{};     // cyrene!! \ 4K
    uint16_t index{};        // only 1 16-bits register
    uint16_t pc{};
    uint16_t stack[16]{}; // 16 level stack
    uint8_t sp{};
    uint8_t delayTimer{};
    uint8_t soundTimer{};
    uint8_t keypad[16]{};
    uint32_t video[64 * 32]{};
    uint16_t opcode;

    Chip8(); // declare constructor
    void LoadROM(char const *filename);

};
