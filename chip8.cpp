#include <fstream> // read or write from file
#include "chip8.hpp"

const unsigned int START_ADDR = 0x200;
const unsigned int FONTSET_START_ADDR = 0x50;
const unsigned int FONTSET_SIZE = 80;

Chip8::Chip8()
{
    // Init PC
    pc = START_ADDR;

    // Load font into mem
    for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
    {
        mem[FONTSET_START_ADDR + i] = fontset[i];
    }
}

void Chip8::LoadROM(char const *filename)
{
    // open file in binary, move fp to end (ate)
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open())
    {
        // allocate heap buffer size of whole file
        std::streampos size = file.tellg();
        char *buffer = new char[size];

        // move fp to beginning and fill buffer
        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        // load ROM into chip8 memory, starting at 0x200
        for (long i = 0; i < size; ++i)
        {
            mem[START_ADDR + i] = buffer[i];
        }

        // rom -> buffer -> mem, therefore free buffer once done
        delete[] buffer;
    }
};

// Chip8 Standard Font Set
uint8_t fontset[FONTSET_SIZE] =
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// OPCODE INSTRUCTIONS: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

/**
 * Clear the display
 */
void Chip8::OP_00E0()
{
    memset(video, 0, sizeof(video));
}

/**
 * Return from a subroutine
 * Interpreter sets PC to address at top of stack, then subtracts 1 from the stack pointer
 */
void Chip8::OP_00EE()
{
    --sp;           // go back to the pc we saved the return address at
    pc = stack[sp]; // set pc to what we saved at that initial pc
}

/**
 * Jump to address
 * Sets PC to nnn
 */
void Chip8::OP_1nnn()
{
    uint16_t addr = opcode & 0x0FFFu; // mask the address (exlude 1 which is code for the jump)

    pc = addr;
}

/**
 * Call soubroutine at nnn.
 */
void Chip8::OP_2nnn()
{
    uint16_t addr = opcode & 0x0FFFu; // mask the address (exlude 2 which is code for the call)

    stack[sp] = pc; // save current pc at the sp
    ++sp;           // go to next sp

    pc = addr;
}

/**
 * Skips next instruction if Vx=kk
 */
void Chip8::OP_3xkk()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    if (registers[Vx] == byte)
    {
        pc += 2; // increment pc by 2 b/c of addressing
    }
}

/**
 * Skip next instruction if Vx!=kk
 */
void Chip8::OP_4xkk()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    if (registers[Vx] != byte)
    {
        pc += 2; // increment pc by 2 b/c of addressing
    }
}