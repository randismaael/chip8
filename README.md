# CHIP-8 Emulator

A CHIP-8 interpreter/emulator written in C++, using SDL2 for graphics and input. CHIP-8 is a simple, interpreted programming language from the 1970s designed for making games on 8-bit microcomputers. Implementing it is a classic first step into emulator development, covering the fetch-decode-execute cycle, memory-mapped I/O, and low-level bit manipulation.

## Features

- Full CHIP-8 instruction set (35 opcodes) implemented via function-pointer dispatch tables
- 4KB addressable memory with the standard built-in hex fontset (0-F) loaded at `0x50`
- 64x32 monochrome display, rendered through SDL2 and scalable to any window size
- 16-key hex keypad input
- Delay and sound timers, decremented once per cycle
- Adjustable emulation speed via a configurable cycle delay
- Proper sprite drawing with screen wrap-around and collision (`VF`) detection

## Project Structure

```
.
├── main.cpp        # Entry point: SDL window setup, ROM loading, main emulation loop
├── chip8.cpp        # Chip8 class: memory, registers, opcode implementations, cycle logic
├── chip8.hpp        # Chip8 class declaration
├── platform.hpp     # SDL2 window/renderer wrapper: input polling, display updates
└── README.md
```

## How It Works

- **Memory (`mem[4096]`):** CHIP-8 programs are loaded starting at `0x200`; the built-in fontset lives at `0x50`.
- **Registers:** 16 general-purpose 8-bit registers (`V0`-`VF`), a 16-bit index register (`I`), and a 16-bit program counter (`PC`). `VF` doubles as a flag register for carry/borrow/collision.
- **Stack:** A 16-level call stack with its own stack pointer, used by `CALL`/`RET` opcodes.
- **Opcode dispatch:** Each opcode's first nibble indexes into a top-level function-pointer table. Opcodes starting with `0`, `8`, `E`, and `F` fan out into their own sub-tables keyed on the last nibble (or last byte, for `F`), since those families pack multiple instructions under one leading digit.
- **Cycle:** Each `Cycle()` call fetches a 2-byte opcode from memory, advances the PC, dispatches to the matching handler, and decrements the delay/sound timers.

## Building

Requires a C++ compiler with SDL2 development libraries installed.

**Install SDL2:**
```bash
# Debian/Ubuntu
sudo apt install libsdl2-dev

# macOS
brew install sdl2
```

**Compile:**
```bash
g++ -std=c++17 main.cpp chip8.cpp -o chip8 $(sdl2-config --cflags --libs)
```

Adjust the compiler invocation to match your actual build setup. A `Makefile` or `CMakeLists.txt` can replace this if you have one.

## Usage

```bash
./chip8 <Scale> <Delay> <ROM>
```

- **Scale:** Integer window scale factor (the native display is 64x32; a scale of `10` gives a 640x320 window).
- **Delay:** Cycle delay in milliseconds, controlling emulation speed (lower runs faster). Most ROMs run well around `1`-`3`.
- **ROM:** Path to a CHIP-8 ROM file (`.ch8`).

**Example:**
```bash
./chip8 10 2 roms/PONG.ch8
```

### Controls

CHIP-8 uses a 16-key hex keypad (`0`-`F`). This emulator maps it to the keyboard as:

| CHIP-8 Key | Keyboard | CHIP-8 Key | Keyboard |
|:---:|:---:|:---:|:---:|
| `0` | `X` | `8` | `S` |
| `1` | `1` | `9` | `D` |
| `2` | `2` | `A` | `Z` |
| `3` | `3` | `B` | `C` |
| `4` | `Q` | `C` | `4` |
| `5` | `W` | `D` | `R` |
| `6` | `E` | `E` | `F` |
| `7` | `A` | `F` | `V` |

`Esc` quits the emulator.

## Known Limitations / Notes

- Timers are decremented once per emulated cycle rather than on a real 60Hz clock, so overall timing is tied to the `Delay` argument rather than wall-clock time.
- No audio output is currently wired up for the sound timer (it counts down but doesn't trigger a beep).
- ROM files are loaded with no bounds checking against the 4KB memory limit. Very large ROM files could overrun memory.

## Resources

- [Cowgod's CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM), the opcode reference used throughout this implementation
- [Austin Morlan's "Building a CHIP-8 Emulator"](https://austinmorlan.com/posts/chip8_emulator/), the tutorial this project follows

## License

MIT (or update to whatever you prefer)
