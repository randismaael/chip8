// read or write from file
#include <fstream>
#include "chip8.hpp"
#include <iostream>

const unsigned int START_ADDR = 0x200;
const unsigned int FONTSET_START_ADDR = 0x50;
const unsigned int FONTSET_SIZE = 80;

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

Chip8::Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
    // Init PC
    pc = START_ADDR;

    // Load font into mem
    for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
    {
        mem[FONTSET_START_ADDR + i] = fontset[i];
    }

    // Random byte
    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

    // locate opcode addresses, fill table in
    table[0x0] = &Chip8::Table0;
    table[0x1] = &Chip8::OP_1nnn;
    table[0x2] = &Chip8::OP_2nnn;
    table[0x3] = &Chip8::OP_3xkk;
    table[0x4] = &Chip8::OP_4xkk;
    table[0x5] = &Chip8::OP_5xy0;
    table[0x6] = &Chip8::OP_6xkk;
    table[0x7] = &Chip8::OP_7xkk;
    table[0x8] = &Chip8::Table8;
    table[0x9] = &Chip8::OP_9xy0;
    table[0xA] = &Chip8::OP_Annn;
    table[0xB] = &Chip8::OP_Bnnn;
    table[0xC] = &Chip8::OP_Cxkk;
    table[0xD] = &Chip8::OP_Dxyn;
    table[0xE] = &Chip8::TableE;
    table[0xF] = &Chip8::TableF;

    // since these values can vary, guard against invalid opcodes
    for (size_t i = 0; i <= 0xE; i++)
    {
        table0[i] = &Chip8::OP_NULL;
        table8[i] = &Chip8::OP_NULL;
        tableE[i] = &Chip8::OP_NULL;
    }

    for (size_t i = 0; i <= 0x65; i++)
    {
        tableF[i] = &Chip8::OP_NULL;
    }

    table0[0x0] = &Chip8::OP_00E0;
    table0[0xE] = &Chip8::OP_00EE;

    table8[0x0] = &Chip8::OP_8xy0;
    table8[0x1] = &Chip8::OP_8xy1;
    table8[0x2] = &Chip8::OP_8xy2;
    table8[0x3] = &Chip8::OP_8xy3;
    table8[0x4] = &Chip8::OP_8xy4;
    table8[0x5] = &Chip8::OP_8xy5;
    table8[0x6] = &Chip8::OP_8xy6;
    table8[0x7] = &Chip8::OP_8xy7;
    table8[0xE] = &Chip8::OP_8xyE;

    tableE[0x1] = &Chip8::OP_ExA1;
    tableE[0xE] = &Chip8::OP_Ex9E;

    tableF[0x07] = &Chip8::OP_Fx07;
    tableF[0x0A] = &Chip8::OP_Fx0A;
    tableF[0x15] = &Chip8::OP_Fx15;
    tableF[0x18] = &Chip8::OP_Fx18;
    tableF[0x1E] = &Chip8::OP_Fx1E;
    tableF[0x29] = &Chip8::OP_Fx29;
    tableF[0x33] = &Chip8::OP_Fx33;
    tableF[0x55] = &Chip8::OP_Fx55;
    tableF[0x65] = &Chip8::OP_Fx65;
}

/**
 * Lookup table for opcodes beginning with 0x0.
 * Uses the last nibble of the opcode to call the correct function.
 */
void Chip8::Table0()
{
    uint8_t idx = opcode & 0x000Fu;
    ((*this).*(table0[idx]))();
}

/**
 * Lookup table for opcodes beginning with 0x8.
 * Uses the last nibble of the opcode to call the correct function.
 */
void Chip8::Table8()
{
    ((*this).*(table8[opcode & 0x000Fu]))();
}

/**
 * Lookup table for opcodes beginning with 0xE.
 * Uses the last nibble of the opcode to call the correct function.
 */
void Chip8::TableE()
{
    ((*this).*(tableE[opcode & 0x000Fu]))();
}

/**
 * Lookup table for opcodes beginning with 0xF.
 * Uses the last byte of the opcode to call the correct function.
 */
void Chip8::TableF()
{
    ((*this).*(tableF[opcode & 0x00FFu]))();
}

/**
 * Opens a ROM file and loads its contents into Chip8 memory starting at 0x200.
 */
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

// OPCODE INSTRUCTIONS: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

/**
 * Placeholder used when an invalid opcode is hit
 */
void Chip8::OP_NULL()
{
    // do nothing - invalid opcode
}

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
    uint16_t addr = opcode & 0xFFFu; // mask the address (exlude 2, the opcode)

    stack[sp] = pc; // save current pc at the sp
    ++sp;           // go to next sp

    pc = addr;
}

/**
 * Skips next instruction if Vx=kk
 */
void Chip8::OP_3xkk()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu; // x is an index into the registers arr (0-15)
    uint8_t byte = opcode & 0xFFu;

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
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;
    uint8_t byte = opcode & 0xFFu;

    if (registers[Vx] != byte)
    {
        pc += 2; // increment pc by 2 b/c of addressing
    }
}

/**
 * Skip next instruction if Vx = Vy
 * The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2
 */
void Chip8::OP_5xy0()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;
    uint8_t Vy = (opcode >> 4u) & 0x0Fu;

    if (registers[Vx] == registers[Vy])
    {
        pc += 2;
    }
}

/**
 * Set Vx = kk
 * The interpreter puts the value kk into register Vx
 */
void Chip8::OP_6xkk()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;
    uint8_t byte = opcode & 0xFFu;

    registers[Vx] = byte;
}

/**
 * Set Vx = Vx + kk
 * Adds the value kk to the value of register Vx, then stores the result in Vx
 */
void Chip8::OP_7xkk()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;
    uint8_t byte = opcode & 0xFFu;

    registers[Vx] += byte;
}

/**
 * Set Vx = Vy
 * Stores the value of register Vy in register Vx
 */
void Chip8::OP_8xy0()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;
    uint8_t Vy = (opcode >> 4u) & 0x0Fu;

    registers[Vx] = registers[Vy];
}

/**
 * Set Vx = Vx OR Vy
 * Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx
 * A bitwise OR compares the corrseponding bits from two values.
 * If either bit is 1, then the same bit in the result is also 1. Otherwise, it is 0.
 */
void Chip8::OP_8xy1()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;
    uint8_t Vy = (opcode >> 4u) & 0x0Fu;

    registers[Vx] = (registers[Vx] | registers[Vy]);
}

/**
 * Set Vx = Vx AND Vy
 * Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
 * A bitwise AND compares the corrseponding bits from two values.
 * If both bits are 1, then the same bit in the result is also 1. Otherwise, it is 0.
 */
void Chip8::OP_8xy2()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;
    uint8_t Vy = (opcode >> 4u) & 0x0Fu;

    registers[Vx] = registers[Vx] & registers[Vy];
}

/**
 * Set Vx = Vx XOR Vy.
 * Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx.
 * An exclusive OR compares the corresponding bits from two values, and if the bits are not both the same,
 * then the corresponding bit in the result is set to 1. Otherwise, it is 0.
 */
void Chip8::OP_8xy3()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;
    uint8_t Vy = (opcode >> 4u) & 0x0Fu;

    registers[Vx] = registers[Vx] ^ registers[Vy];
}

/**
 * Set Vx = Vx + Vy, set VF = carry.
 * The values of Vx and Vy are added together.
 * If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0.
 * Only the lowest 8 bits of the result are kept, and stored in Vx.
 */
void Chip8::OP_8xy4()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;
    uint8_t Vy = (opcode >> 4u) & 0x0Fu;

    uint16_t sum = registers[Vx] + registers[Vy];

    registers[0xF] = (sum > 255U) ? 1 : 0;
    // lowest 8 bits of the 16 bit sum
    registers[Vx] = sum & 0xFFu;
}

/**
 * Set Vx = Vx - Vy, set VF = NOT borrow.
 * If Vx > Vy, then VF is set to 1, otherwise 0.
 * Then Vy is subtracted from Vx, and the results stored in Vx.
 */
void Chip8::OP_8xy5()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;
    uint8_t Vy = (opcode >> 4u) & 0x0Fu;

    registers[0xF] = (registers[Vx] > registers[Vy]) ? 1 : 0;

    registers[Vx] -= registers[Vy];
}

/**
 * Set Vx = Vx SHR 1.
 * If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0.
 * Then Vx is divided by 2.
 */
void Chip8::OP_8xy6()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;

    // LSB can only be 0 or 1
    registers[0xF] = registers[Vx] & 0x1u;
    registers[Vx] >>= 1;
}

/**
 * Set Vx = Vy - Vx, set VF = NOT borrow.
 * If Vy > Vx, then VF is set to 1, otherwise 0.
 * Then Vx is subtracted from Vy, and the results stored in Vx.
 */
void Chip8::OP_8xy7()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;
    uint8_t Vy = (opcode >> 4u) & 0x0Fu;

    registers[0xF] = (registers[Vy] > registers[Vx]) ? 1 : 0;

    registers[Vx] = registers[Vy] - registers[Vx];
}

/**
 * Set Vx = Vx SHL 1.
 * If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0.
 * Then Vx is multiplied by 2.
 */
void Chip8::OP_8xyE()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;

    registers[0xF] = (registers[Vx] >> 7u) & 0x1u;
    registers[Vx] <<= 1;
}

/**
 * Skip next instruction if Vx != Vy.
 * The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
 */
void Chip8::OP_9xy0()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;
    uint8_t Vy = (opcode >> 4u) & 0x0Fu;

    if (registers[Vx] != registers[Vy])
    {
        pc += 2;
    }
}

/**
 * Set I = nnn.
 * The value of index register I is set to nnn, a memory address.
 */
void Chip8::OP_Annn()
{
    uint16_t address = opcode & 0xFFFu;

    index = address;
}

/**
 * Jump to location nnn + V0.
 * The program counter is set to nnn plus the value of V0.
 */
void Chip8::OP_Bnnn()
{
    uint16_t address = opcode & 0xFFFu;

    pc = address + registers[0];
}

/**
 * Set Vx = random byte AND kk.
 * The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk.
 * The results are stored in Vx.
 */
void Chip8::OP_Cxkk()
{
    uint8_t Vx = (opcode >> 8u) & 0xFu;
    uint8_t byte = opcode & 0xFFu;

    registers[Vx] = randByte(randGen) & byte; // me mention....
}

/**
 * Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
 * The interpreter reads n bytes from memory, starting at the address stored in I.
 * These bytes are then displayed as sprites on screen at coordinates (Vx, Vy).
 * Sprites are XORed onto the existing screen.
 * If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0.
 * If the sprite is positioned so part of it is outside the coordinates of the display, it wraps around to the opposite side of the screen.
 */
void Chip8::OP_Dxyn()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;
    uint8_t Vy = (opcode >> 4u) & 0x0Fu;
    uint8_t height = opcode & 0xFu;

    // wrap around
    uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
    uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

    registers[0xF] = 0;

    for (unsigned int row = 0; row < height; ++row)
    {
        uint8_t spriteByte = mem[index + row];

        for (unsigned int col = 0; col < 8; ++col)
        {
            uint8_t spritePixel = spriteByte & (0x80u >> col); // pixel on (1)
            uint32_t *screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

            if (spritePixel)
            {
                // if trying to draw on a lit pixel
                if (*screenPixel == 0xFFFFFFFF)
                {
                    registers[0xF] = 1;
                }

                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}

/**
 * Skip next instruction if key with the value of Vx is pressed.
 * Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position, PC is increased by 2.
 */
void Chip8::OP_Ex9E()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;

    if (keypad[registers[Vx]])
    {
        pc += 2;
    }
}

/**
 * Skip next instruction if key with the value of Vx is not pressed.
 * Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position, PC is increased by 2.
 */
void Chip8::OP_ExA1()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;

    if (!keypad[registers[Vx]])
    {
        pc += 2;
    }
}

/**
 * Set Vx = delay timer value.
 * The value of DT is placed into Vx.
 */
void Chip8::OP_Fx07()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;

    registers[Vx] = delayTimer;
}

/**
 * Wait for a key press, store the value of the key in Vx.
 * All execution stops until a key is pressed, then the value of that key is stored in Vx.
 */
void Chip8::OP_Fx0A()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;

    for (int i = 0; i < 16; ++i)
    {
        if (keypad[i])
        {
            registers[Vx] = i;
            return;
        }
    }
    // keep rerunning until key is pressed
    pc -= 2;
}

/**
 * Set delay timer = Vx.
 * DT is set equal to the value of Vx.
 */
void Chip8::OP_Fx15()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;

    delayTimer = registers[Vx];
}

/**
 * Set sound timer = Vx.
 * ST is set equal to the value of Vx.
 */
void Chip8::OP_Fx18()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;

    soundTimer = registers[Vx];
}

/**
 * Set I = I + Vx.
 * The values of I and Vx are added, and the results are stored in I.
 */
void Chip8::OP_Fx1E()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;

    index += registers[Vx];
}

/**
 * Set I = location of sprite for digit Vx.
 * The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
 */
void Chip8::OP_Fx29()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;

    // each character is 5 bytes
    index = FONTSET_START_ADDR + (5 * registers[Vx]);
}

/**
 * Store BCD representation of Vx in memory locations I, I+1, and I+2.
 * The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I,
 * the tens digit at location I+1, and the ones digit at location I+2.
 */
void Chip8::OP_Fx33()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;
    uint8_t value = registers[Vx];

    // ones
    mem[index + 2] = value % 10;
    value /= 10;

    // tens
    mem[index + 1] = value % 10;
    value /= 10;

    // hundreds
    mem[index] = value % 10;
}

/**
 * Store registers V0 through Vx in memory starting at location I.
 * The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.
 */
void Chip8::OP_Fx55()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;

    for (uint8_t i = 0; i <= Vx; ++i)
    {
        mem[index + i] = registers[i];
    }
}

/**
 * Read registers V0 through Vx from memory starting at location I.
 * The interpreter reads values from memory starting at location I into registers V0 through Vx.
 */
void Chip8::OP_Fx65()
{
    uint8_t Vx = (opcode >> 8u) & 0x0Fu;

    for (uint8_t i = 0; i <= Vx; ++i)
    {
        registers[i] = mem[index + i];
    }
}

/**
 * Fetch, decode, and execute one opcode cycle.
 * Fetches the next 2-byte opcode from memory, increments the PC,
 * executes the corresponding opcode function, and decrements the timers.
 */
void Chip8::Cycle()
{
    // Fetch
    // Opcodes are 2 bytes but memory is 1 byte per address, so combine two consecutive bytes
    opcode = ((uint16_t)mem[pc] << 8u) | (uint16_t)mem[pc + 1];
    pc += 2;

    // Use first nibble as index into function pointer table, decode and execute
    ((*this).*(table[(opcode >> 12u) & 0xF]))();

    // Count down every cycle
    if (delayTimer > 0)
    {
        --delayTimer;
    }

    if (soundTimer > 0)
    {
        --soundTimer;
    }
}
