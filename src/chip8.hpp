#pragma once

#include <cstdint>

// constants
const unsigned int START_ADDRESS = 0x200; // first 512 bytes reserved for interpreter
const unsigned int FONTSET_SIZE = 80; // size of all 16 characters (0-F) in bytes
const unsigned int FONTSET_START_ADDRESS = 0x50; // location of fontset
const unsigned int VIDEO_WIDTH = 64;
const unsigned int VIDEO_HEIGHT = 32;

class Chip8
{
public:
    Chip8();
    void cycle();
    bool loadROM(const char* filename);
    void reset();

    uint8_t registers[16]{}; // V
    uint8_t memory[4096]{}; // memory
    uint16_t index{}; // I
    uint16_t pc{}; // program counter
    uint16_t stack[16]{}; // stack
    uint8_t sp{}; // stack pointer
    uint8_t delayTimer{}; // delay timer
    uint8_t soundTimer{}; // sound timer
    uint8_t keypad[16]{}; // keypad
    uint32_t video[64 * 32]{}; // framebuffer
    uint16_t opcode; // current opcode

private:
    // instruction decoding function pointer type
    typedef void (Chip8::*Chip8Func)();

    // instruction tables
    Chip8Func table[0xF + 1];
    Chip8Func table0[0xE + 1];
    Chip8Func table8[0xE + 1];
    Chip8Func tableE[0xE + 1];
    Chip8Func tableF[0x65 + 1];

    // helper functions
    void initInstructions();
    uint8_t getRandomByte();

    // table functions
    void Table0();
    void Table8();
    void TableE();
    void TableF();

    // instructions
    void op00E0();
    void op00EE();
    void op1NNN();
    void op2NNN();
    void op3XKK();
    void op4XKK();
    void op5XY0();
    void op6XKK();
    void op7XKK();
    void op8XY0();
    void op8XY1();
    void op8XY2();
    void op8XY3();
    void op8XY4();
    void op8XY5();
    void op8XY6();
    void op8XY7();
    void op8XYE();
    void op9XY0();
    void opANNN();
    void opBNNN();
    void opCXNN();
    void opDXYN();
    void opEX9E();
    void opEXA1();
    void opFX07();
    void opFX0A();
    void opFX15();
    void opFX18();
    void opFX1E();
    void opFX29();
    void opFX33();
    void opFX55();
    void opFX65();
    void opNULL();

    // opcode helpers
    uint8_t getVx() const { return (opcode & 0x0F00) >> 8; }
    uint8_t getVy() const { return (opcode & 0x00F0) >> 4; }
    uint8_t getKK() const { return opcode & 0x00FF; }
    uint16_t getNNN() const { return opcode & 0x0FFF; }
};
