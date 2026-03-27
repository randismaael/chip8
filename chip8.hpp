#include <cstdint>
#include <chrono>
#include <random>
#include <stdio.h>
#include <SDL2/SDL.h>       //homebrew SDL2
#include <SDL2/SDL_video.h> //for window creation
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

    void OP_NULL(); // Null guard

    // type alias for function pointer that takes no arguments and returns void
    typedef void (Chip8::*Chip8Func)();

    // indexed  by last nibble (one), 0-F
    Chip8Func table[0x10];
    Chip8Func table0[0x10]; // 0xE + 1
    Chip8Func table8[0x10]; // 0xE + 1
    Chip8Func tableE[0x10]; // 0xE + 1

    // indexed by last byte (two), highest 0x65
    Chip8Func tableF[0x100]; // 0x65 + 1

    // declare
    void Table();
    void Table0();
    void Table8();
    void TableF();
    void TableE();

    // Fetch
    void Cycle();
};

/**
 * Platform layer using SDL2 for window creation, rendering, and input handling.
 * Allows for cross-platform emulation.
 * Handles mapping of keyboard input to CHIP-8 keypad and rendering video[] to screen.
 */
class Platform
{
public:
    // Constructor
    Platform(char const *title, int windowWidth, int windowHeight, int textureWidth, int textureHeight)
    {
        SDL_Init(SDL_INIT_VIDEO);

        window = SDL_CreateWindow(title, 0, 0, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight);
    }

    // Destructor
    ~Platform()
    {
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void Update(void const *buffer, int pitch)
    {
        SDL_UpdateTexture(texture, nullptr, buffer, pitch);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    bool ProcessInput(uint8_t *keys)
    {
        bool quit = false;

        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
            {
                quit = true;
            }
            break;

            case SDL_KEYDOWN:
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                {
                    quit = true;
                }
                break;
                case SDLK_x:
                {
                    keys[0] = 1;
                }
                break;

                case SDLK_1:
                {
                    keys[1] = 1;
                }
                break;

                case SDLK_2:
                {
                    keys[2] = 1;
                }
                break;

                case SDLK_3:
                {
                    keys[3] = 1;
                }
                break;

                case SDLK_q:
                {
                    keys[4] = 1;
                }
                break;

                case SDLK_w:
                {
                    keys[5] = 1;
                }
                break;

                case SDLK_e:
                {
                    keys[6] = 1;
                }
                break;

                case SDLK_a:
                {
                    keys[7] = 1;
                }
                break;

                case SDLK_s:
                {
                    keys[8] = 1;
                }
                break;

                case SDLK_d:
                {
                    keys[9] = 1;
                }
                break;

                case SDLK_z:
                {
                    keys[0xA] = 1;
                }
                break;

                case SDLK_c:
                {
                    keys[0xB] = 1;
                }
                break;

                case SDLK_4:
                {
                    keys[0xC] = 1;
                }
                break;

                case SDLK_r:
                {
                    keys[0xD] = 1;
                }
                break;

                case SDLK_f:
                {
                    keys[0xE] = 1;
                }
                break;

                case SDLK_v:
                {
                    keys[0xF] = 1;
                }
                break;
                }
            }
            case SDL_KEYUP:
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_x:
                {
                    keys[0] = 0;
                }
                break;

                case SDLK_1:
                {
                    keys[1] = 0;
                }
                break;

                case SDLK_2:
                {
                    keys[2] = 0;
                }
                break;

                case SDLK_3:
                {
                    keys[3] = 0;
                }
                break;

                case SDLK_q:
                {
                    keys[4] = 0;
                }
                break;

                case SDLK_w:
                {
                    keys[5] = 0;
                }
                break;

                case SDLK_e:
                {
                    keys[6] = 0;
                }
                break;

                case SDLK_a:
                {
                    keys[7] = 0;
                }
                break;

                case SDLK_s:
                {
                    keys[8] = 0;
                }
                break;

                case SDLK_d:
                {
                    keys[9] = 0;
                }
                break;

                case SDLK_z:
                {
                    keys[0xA] = 0;
                }
                break;

                case SDLK_c:
                {
                    keys[0xB] = 0;
                }
                break;

                case SDLK_4:
                {
                    keys[0xC] = 0;
                }
                break;

                case SDLK_r:
                {
                    keys[0xD] = 0;
                }
                break;

                case SDLK_f:
                {
                    keys[0xE] = 0;
                }
                break;

                case SDLK_v:
                {
                    keys[0xF] = 0;
                }
                break;
                }
            }
            break;
            }
        }

        return quit;
    }

private:
    SDL_Window *window{};
    SDL_Renderer *renderer{};
    SDL_Texture *texture{};
};

