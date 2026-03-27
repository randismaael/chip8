// time related functions, helps with random number generator
#include <chrono>
// input/output library
#include <iostream>

#include "chip8.hpp"
#include "platform.hpp"

#include "chip8.cpp"

int main(int argc, char **argv)
{
    // error msg
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
        std::exit(EXIT_FAILURE);
    }

    int videoScale = std::stoi(argv[1]);
    int cycleDelay = std::stoi(argv[2]);
    char const *romFilename = argv[3];

    // create sdl window
    Platform platform("CHIP-8 Emulator", VIDEO_WIDTH * videoScale, VIDEO_HEIGHT * videoScale, VIDEO_WIDTH, VIDEO_HEIGHT);

    Chip8 chip8;
    chip8.LoadROM(romFilename);

    // how many bytes wide one row of pixels is, needed for SDL to navigate pixel buffer
    int videoPitch = sizeof(chip8.video[0]) * VIDEO_WIDTH;

    // record curr time
    auto lastCycleTime = std ::chrono::high_resolution_clock::now();

    bool quit = false;

    while (!quit)
    {
        quit = platform.ProcessInput(chip8.keypad);

        auto currTime = std::chrono::high_resolution_clock::now();
        // calculate time elapsed in seconds
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currTime - lastCycleTime).count();

        // changing clock cycle time to match ROM file rather than CPU
        if (dt > cycleDelay)
        {
            lastCycleTime = currTime;
            chip8.Cycle();
            platform.Update(chip8.video, videoPitch);
        }
    }
    return;
}