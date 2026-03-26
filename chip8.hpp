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

    // list of OPCODES
    void OP_00E0(); // CLS - clear screen
    void OP_00EE(); // RET - return from subroutine
    void OP_1nnn(); // JP - jump to addr at nnn
    void OP_2nnn(); // CALL - call subroutine at nnn
    void OP_3xkk(); // SE - Skip if Vx=kk
    void OP_4xkk(); // SNE - Skip if Vx!=kk
    void OP_5xy0(); // SE - Skip next instruction if Vx = Vy
    void OP_6xkk(); // LD - Set Vx = kk
    void OP_7xkk(); // ADD - Set Vx = Vx + kk
    void OP_8xy0(); // LD - Set Vx = Vy
    void OP_8xy1(); // OR - Set Vx = Vx OR Vy
    void OP_8xy2(); // AND - Set Vx = Vx AND Vy
    void OP_8xy3(); // XOR - Set Vx = Vx XOR Vy
    void OP_8xy4(); // ADD - Set Vx = Vx + Vy, set VF = carry
    void OP_8xy5(); // SUB - Set Vx = Vx - Vy, set VF = NOT borrow
    void OP_8xy6(); // SHR - Set Vx = Vx SHR 1
    void OP_8xy7(); // SUBN - Set Vx = Vy - Vx, set VF = NOT borrow
    void OP_8xyE(); // SHL - Set Vx = Vx SHL 1
    void OP_9xy0(); // SNE - Skip next instruction if Vx != Vy
    void OP_Annn(); // LD - Set I = nnn
    void OP_Bnnn(); // JP - Jump to location nnn + V0
    void OP_Cxkk(); // RND - Set Vx = random byte AND kk
    void OP_Dxyn(); // DRW - Display n-byte sprite at (Vx, Vy), set VF = collision
    void OP_Ex9E(); // SKP - Skip next instruction if key Vx is pressed
    void OP_ExA1(); // SKNP - Skip next instruction if key Vx is not pressed
    void OP_Fx07(); // LD - Set Vx = delay timer value
    void OP_Fx0A(); // LD - Wait for key press, store value in Vx
    void OP_Fx15(); // LD - Set delay timer = Vx
    void OP_Fx18(); // LD - Set sound timer = Vx
    void OP_Fx1E(); // ADD - Set I = I + Vx
    void OP_Fx29(); // LD - Set I = location of sprite for digit Vx
    void OP_Fx33(); // LD - Store BCD representation of Vx in I, I+1, I+2
    void OP_Fx55(); // LD - Store registers V0 through Vx in memory at I
    void OP_Fx65(); // LD - Read registers V0 through Vx from memory at I
};
