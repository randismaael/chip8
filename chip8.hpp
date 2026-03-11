#include <cstdint>
#include <chrono>
#include <random>
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

    Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count())
    {
        randByte = std::uniform_int_distribution<uint8_t>(0, 255U);
    }

    std::default_random_engine randGen;
    std::uniform_int_distribution<uint8_t> randByte; // rand number between 0 and 255

    //list of OPCODES
    void OP_00E0(); //CLS - clear screen
    void OP_00EE(); //RET - return from subroutine
    void OP_1nnn(); //JP - jump to addr at nnn
    void OP_2nnn(); //CALL - call subroutine at nnn
    void OP_3xkk(); //SE - Skip if Vx=kk
    void OP_4xkk(); //SNE - Skip if Vx!=kk
    
};
