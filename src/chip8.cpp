#include "chip8.hpp"

// initialize
void Chip8::reset() {
    pc = START_ADDRESS;
    index = 0;
    sp = 0;
    opcode = 0;
    delayTimer = 0;
    soundTimer = 0;

    // clear registers
    std::fill(std::begin(registers), std::end(registers), 0);
    std::fill(std::begin(stack), std::end(stack), 0);
    std::fill(std::begin(video), std::end(video), 0);
    std::fill(std::begin(keypad), std::end(keypad), 0);
    
    // clear memory
    std::fill(std::begin(memory), std::end(memory), 0);

    // load font into memory
    uint8_t fontset[FONTSET_SIZE] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70,
        0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0,
        0x90, 0x90, 0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0,
        0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40,
        0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0, 0x10, 0xF0,
        0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0,
        0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0,
        0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80
    };

    for (int i = 0; i < FONTSET_SIZE; ++i) {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }
}

// constructor
Chip8::Chip8() {
    srand(static_cast<unsigned int>(time(NULL)));
    initInstructions();
    reset();
}

void Chip8::cycle() {
    // fetch
    opcode = memory[pc] << 8u | memory[pc + 1];

    // increment pc before execution
    pc += 2;

    // decode & execute
    ((*this).*(table[(opcode & 0xF000u) >> 12u]))();

    // decrement delay timer
    if (delayTimer > 0) {
        --delayTimer;
    }

    // decrement sound timer
    if (soundTimer > 0) {
        --soundTimer;
    }
}

// load rom into emulator memory
bool Chip8::loadROM(const char* filename) {
    // read stream as binary
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }

    // get file size
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // check size
    if (size > (4096 - START_ADDRESS)) {
        return false;
    }

    // read file into memory
    file.read(reinterpret_cast<char*>(&memory[START_ADDRESS]), size);

    file.close();
    return true;
}

// ====================
// Instructions
// ====================

// 00E0 - CLS - clear the display
void Chip8::op00E0() {
    // set entire framebuffer to 0
    for (int i = 0; i < 64 * 32; ++i) {
        video[i] = 0;
    }
}

// 00EE - RET - return from a subroutine
void Chip8::op00EE() {
    // set pc to top of stack
    pc = stack[--sp];
}

// 1NNN - JP addr - jump to address NNN
void Chip8::op1NNN() {
    // set pc to NNN
    pc = getNNN();
}

// 2NNN - CALL addr - call subroutine at NNN
void Chip8::op2NNN() {
    uint16_t address = getNNN();

    // push current pc to stack
    stack[sp++] = pc;

    // set pc to NNN
    pc = address;
}

// 3XKK - SE Vx, byte - skip next instruction if Vx == KK
void Chip8::op3XKK() {
    uint8_t Vx = getVx();
    uint8_t KK = getKK();

    if (registers[Vx] == KK) {
        // skip next instruction
        pc += 2;
    }
}

// 4XKK - SNE Vx, byte - skip next instruction if Vx != KK
void Chip8::op4XKK() {
    uint8_t Vx = getVx();
    uint8_t KK = getKK();

    if (registers[Vx] != KK) {
        // skip next instruction
        pc += 2;
    }
}

// 5XY0 - SE Vx, Vy - skip next instruction if Vx == Vy
void Chip8::op5XY0() {
    uint8_t Vx = getVx();
    uint8_t Vy = getVy();

    if (registers[Vx] == registers[Vy]) {
        // skip next instruction
        pc += 2;
    }
}

// 6XKK - LD Vx, byte - set Vx to KK
void Chip8::op6XKK() {
    uint8_t Vx = getVx();
    uint8_t KK = getKK();

    registers[Vx] = KK;
}

// 7XKK - ADD Vx, byte - set Vx to Vx + KK
void Chip8::op7XKK() {
    uint8_t Vx = getVx();
    uint8_t KK = getKK();

    registers[Vx] += KK;
}

// 8XY0 - LD Vx, Vy - set Vx to Vy
void Chip8::op8XY0() {
    uint8_t Vx = getVx();
    uint8_t Vy = getVy();

    registers[Vx] = registers[Vy];
}

// 8XY1 - OR Vx, Vy - set Vx to Vx OR Vy
void Chip8::op8XY1() {
    uint8_t Vx = getVx();
    uint8_t Vy = getVy();

    registers[Vx] |= registers[Vy]; // OR
}

// 8XY2 - AND Vx, Vy - set Vx to Vx AND Vy
void Chip8::op8XY2() {
    uint8_t Vx = getVx();
    uint8_t Vy = getVy();

    registers[Vx] &= registers[Vy]; // AND
}

// 8XY3 - XOR Vx, Vy - set Vx to Vx XOR Vy
void Chip8::op8XY3() {
    uint8_t Vx = getVx();
    uint8_t Vy = getVy();

    registers[Vx] ^= registers[Vy]; // XOR
}

// 8XY4 - ADD Vx, Vy - set Vx to Vx + Vy, set VF to carry
void Chip8::op8XY4() {
    uint8_t Vx = getVx();
    uint8_t Vy = getVy();

    uint8_t flag = ((registers[Vx] + registers[Vy]) > 255U) ? 1 : 0; // check carry
    registers[Vx] = (registers[Vx] + registers[Vy]) & 0xFFu; // add
    registers[0xF] = flag; // set carry flag
}

// 8XY5 - SUB Vx, Vy - set Vx to Vx - Vy, set VF to NOT borrow
void Chip8::op8XY5() {
    uint8_t Vx = getVx();
    uint8_t Vy = getVy();

    uint8_t flag = (registers[Vx] >= registers[Vy]) ? 1 : 0; // check borrow
    registers[Vx] -= registers[Vy]; // subtract
    registers[0xF] = flag; // set carry flag
}

// 8XY6 - SHR Vx - set Vx to Vx SHR 1
void Chip8::op8XY6() {
    uint8_t Vx = getVx();

    // save least significant bit in VF
    registers[0xF] = registers[Vx] & 0x01;

    // set Vx to Vx SHR 1
    registers[Vx] >>= 1;
}

// 8XY7 - SUBN Vx, Vy - set Vx to Vy - Vx, set VF to NOT borrow
void Chip8::op8XY7() {
    uint8_t Vx = getVx();
    uint8_t Vy = getVy();

    uint8_t flag = (registers[Vx] >= registers[Vy]) ? 1 : 0; // check borrow
    registers[Vx] = registers[Vy] - registers[Vx]; // subtract
    registers[0xF] = flag; // set carry flag
}

// 8XYE - SHL Vx - set Vx to Vx SHL 1
void Chip8::op8XYE() {
    uint8_t Vx = getVx();

    // save most significant bit in VF
    registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

    // set Vx to Vx SHL 1
    registers[Vx] <<= 1;
}

// 9XY0 - SNE Vx, Vy - skip next instruction if Vx != Vy
void Chip8::op9XY0() {
    uint8_t Vx = getVx();
    uint8_t Vy = getVy();

    if (registers[Vx] != registers[Vy]) {
        // skip next instruction
        pc += 2;
    }
}

// ANNN - LD I, addr - set I to addr
void Chip8::opANNN() {
    index = getNNN();
}

// BNNN - JP V0, addr - set pc to V0 + addr
void Chip8::opBNNN() {
    uint16_t address = getNNN();
    pc = registers[0] + address;
}

// CXNN - RND Vx, byte - set Vx to random byte AND nn
void Chip8::opCXNN() {
    uint8_t Vx = getVx();
    uint8_t NN = getKK();

    registers[Vx] = getRandomByte() & NN; // AND
}

// DXYN - DRW Vx, Vy, nibble - display n-byte sprite starting at memory location I at (Vx, Vy), set VF to collision
void Chip8::opDXYN() {
    uint8_t Vx = getVx();
    uint8_t Vy = getVy();
    uint8_t height = opcode & 0x000Fu;

    uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
    uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

    // clear collision
    registers[0xF] = 0; 

    for (unsigned int row = 0; row < height; ++row) {
        // break if out of bounds
        if (yPos + row >= VIDEO_HEIGHT) break;

        uint8_t spriteByte = memory[index + row];

        for (unsigned int col = 0; col < 8; ++col) {
            // break it out of bounds
            if (xPos + col >= VIDEO_WIDTH) break;

            uint8_t spritePixel = spriteByte & (0x80 >> col);
            uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

            // collision detection
            if (spritePixel) {
                if (*screenPixel == 0xFFFFFFFF) {
                    registers[0xF] = 1;
                }
                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}

// EX9E - SKP Vx - skip next instruction if key with the value of Vx is pressed
void Chip8::opEX9E() {
    uint8_t Vx = getVx();
    uint8_t key = registers[Vx];

    if(keypad[key]) {
        // skip next instruction
        pc += 2;
    }
}

// EXA1 - SKNP Vx - skip next instruction if key with the value of Vx is not pressed
void Chip8::opEXA1() {
    uint8_t Vx = getVx();
    uint8_t key = registers[Vx];

    if(!keypad[key]) {
        // skip next instruction
        pc += 2;
    }
}

// FX07 - LD Vx, DT - set Vx to delay timer value
void Chip8::opFX07() {
    uint8_t Vx = getVx();
    registers[Vx] = delayTimer;
}

// FX0A - LD Vx, K - wait for a key press, store the value of the key in Vx
void Chip8::opFX0A() {
    uint8_t Vx = getVx();

    bool keyPressed = false;

    // loop all keys and check if one is pressed
    for (uint8_t key = 0; key < 16; ++key) {
        if (keypad[key]) {
            registers[Vx] = key;
            keyPressed = true;
            break;
        }
    }

    if (!keyPressed) {
        // skip next instruction
        pc -= 2;
    }
}

// FX15 - LD DT, Vx - set delay timer to Vx
void Chip8::opFX15() {
    uint8_t Vx = getVx();
    delayTimer = registers[Vx];
}

// FX18 - LD ST, Vx - set sound timer to Vx
void Chip8::opFX18() {
    uint8_t Vx = getVx();
    soundTimer = registers[Vx];
}

// FX1E - ADD I, Vx - set Index to Index + Vx
void Chip8::opFX1E() {
    uint8_t Vx = getVx();
    index += registers[Vx];
}

// FX29 - LD F, Vx - set I to location of sprite for digit Vx
void Chip8::opFX29() {
    uint8_t Vx = getVx();
    uint8_t digit = registers[Vx];

    // 5 bytes per digit
    index = FONTSET_START_ADDRESS + (digit * 5);
}

// FX33 - LD B, Vx - store BCD representation of Vx in memory locations I, I+1, and I+2
void Chip8::opFX33() {
    uint8_t Vx = getVx();
    uint16_t value = registers[Vx];

    // ones place
    memory[index + 2] = value % 10;
    value /= 10;

    // tens place
    memory[index + 1] = value % 10;
    value /= 10;

    // hundreds place
    memory[index] = value % 10;
}

// FX55 - LD [I], Vx - store registers V0 through Vx in memory starting at location I
void Chip8::opFX55() {
    uint8_t Vx = getVx();

    // store registers V0 through Vx in memory starting at location I
    for (uint8_t i = 0; i <= Vx; ++i) {
        memory[index + i] = registers[i];
    }
}

// FX65 - LD Vx, [I] - read registers V0 through Vx from memory starting at location I
void Chip8::opFX65() {
    uint8_t Vx = getVx();

    // read registers V0 through Vx from memory starting at location I
    for (uint8_t i = 0; i <= Vx; ++i) {
        registers[i] = memory[index + i];
    }
}

// NULL - dummy function for unimplemented opcodes
void Chip8::opNULL() {}

// ====================
// instruction table
// ====================

void Chip8::initInstructions() {
    // set up instruction table
    table[0x0] = &Chip8::Table0; //similar type of pointer
    table[0x1] = &Chip8::op1NNN;
    table[0x2] = &Chip8::op2NNN;
    table[0x3] = &Chip8::op3XKK;
    table[0x4] = &Chip8::op4XKK;
    table[0x5] = &Chip8::op5XY0;
    table[0x6] = &Chip8::op6XKK;
    table[0x7] = &Chip8::op7XKK;
    table[0x8] = &Chip8::Table8; // similar
    table[0x9] = &Chip8::op9XY0;
    table[0xA] = &Chip8::opANNN;
    table[0xB] = &Chip8::opBNNN;
    table[0xC] = &Chip8::opCXNN;
    table[0xD] = &Chip8::opDXYN;
    table[0xE] = &Chip8::TableE; // similar
    table[0xF] = &Chip8::TableF; // similar

    for (size_t i = 0; i <= 0xE; i++)
    {
        table0[i] = &Chip8::opNULL;
        table8[i] = &Chip8::opNULL;
        tableE[i] = &Chip8::opNULL;
    }

    table0[0x0] = &Chip8::op00E0;
    table0[0xE] = &Chip8::op00EE;

    table8[0x0] = &Chip8::op8XY0;
    table8[0x1] = &Chip8::op8XY1;
    table8[0x2] = &Chip8::op8XY2;
    table8[0x3] = &Chip8::op8XY3;
    table8[0x4] = &Chip8::op8XY4;
    table8[0x5] = &Chip8::op8XY5;
    table8[0x6] = &Chip8::op8XY6;
    table8[0x7] = &Chip8::op8XY7;
    table8[0xE] = &Chip8::op8XYE;

    tableE[0x1] = &Chip8::opEXA1;
    tableE[0xE] = &Chip8::opEX9E;

    for (size_t i = 0; i <= 0x65; i++)
    {
        tableF[i] = &Chip8::opNULL;
    }

    tableF[0x07] = &Chip8::opFX07;
    tableF[0x0A] = &Chip8::opFX0A;
    tableF[0x15] = &Chip8::opFX15;
    tableF[0x18] = &Chip8::opFX18;
    tableF[0x1E] = &Chip8::opFX1E;
    tableF[0x29] = &Chip8::opFX29;
    tableF[0x33] = &Chip8::opFX33;
    tableF[0x55] = &Chip8::opFX55;
    tableF[0x65] = &Chip8::opFX65;
}

void Chip8::Table0() {
    (this->*(table0[opcode & 0x000Fu]))();
}

void Chip8::Table8() {
    (this->*(table8[opcode & 0x000Fu]))();
}

void Chip8::TableE() {
    (this->*(tableE[opcode & 0x000Fu]))();
}

void Chip8::TableF() {
    (this->*(tableF[opcode & 0x00FFu]))();
}

// ====================
// helpers
// ====================

// get random byte
uint8_t Chip8::getRandomByte() {
    return static_cast<uint8_t>(rand() % 256);
}