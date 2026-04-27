#include "chip8.hpp"

// initialize
Chip8::Chip8() {
    // seed random
    srand(time(NULL));

    // program counter starts at 0x200
    pc = START_ADDRESS;

    uint8_t fontset[FONTSET_SIZE] = {
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

    // load fonts into memory
    for (int i = 0; i < FONTSET_SIZE; ++i) {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    initInstructions();
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
void Chip8::loadROM(const char* filename) {
    // open stream as binary
    std::ifstream file = std::ifstream(filename, std::ios::binary | std::ios::ate);
    
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open ROM file");
    }

    // get file size
    std::streampos size = file.tellg();

    // create buffer and read file into it
    char* buffer = new char[size];
    file.seekg(0, std::ios::beg);
    file.read(buffer, size);
    file.close();

    // load rom into buffer (after start address)
    for (int i = 0; i < size; ++i) {
        memory[START_ADDRESS + i] = buffer[i];
    }

    // free buffer
    delete[] buffer;
}

// get random byte
uint8_t Chip8::getRandomByte() {
    return static_cast<uint8_t>(rand() % 256);
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
    pc = opcode & 0x0FFF; // bitmask to get last 12 bits
}

// 2NNN - CALL addr - call subroutine at NNN
void Chip8::op2NNN() {
    uint16_t address = opcode & 0x0FFF;

    // push current pc to stack
    stack[sp++] = pc;

    // set pc to NNN
    pc = address;
}

// 3XKK - SE Vx, byte - skip next instruction if Vx == KK
void Chip8::op3XKK() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get KK
    uint8_t KK = opcode & 0x00FF;

    // check if Vx == KK
    if (registers[Vx] == KK) {
        // skip next instruction
        pc += 2;
    }
}

// 4XKK - SNE Vx, byte - skip next instruction if Vx != KK
void Chip8::op4XKK() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get KK
    uint8_t KK = opcode & 0x00FF;

    // check if Vx != KK
    if (registers[Vx] != KK) {
        // skip next instruction
        pc += 2;
    }
}

// 5XY0 - SE Vx, Vy - skip next instruction if Vx == Vy
void Chip8::op5XY0() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get Vy
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    // check if Vx == Vy
    if (registers[Vx] == registers[Vy]) {
        // skip next instruction
        pc += 2;
    }
}

// 6XKK - LD Vx, byte - set Vx to KK
void Chip8::op6XKK() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get KK
    uint8_t KK = opcode & 0x00FF;

    // set Vx to KK
    registers[Vx] = KK;
}

// 7XKK - ADD Vx, byte - set Vx to Vx + KK
void Chip8::op7XKK() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get KK
    uint8_t KK = opcode & 0x00FF;

    // set Vx to Vx + KK
    registers[Vx] += KK;
}

// 8XY0 - LD Vx, Vy - set Vx to Vy
void Chip8::op8XY0() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get Vy
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    // set Vx to Vy
    registers[Vx] = registers[Vy];
}

// 8XY1 - OR Vx, Vy - set Vx to Vx OR Vy
void Chip8::op8XY1() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get Vy
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    // set Vx to Vx OR Vy
    registers[Vx] |= registers[Vy];
}

// 8XY2 - AND Vx, Vy - set Vx to Vx AND Vy
void Chip8::op8XY2() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get Vy
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    // set Vx to Vx AND Vy
    registers[Vx] &= registers[Vy];
}

// 8XY3 - XOR Vx, Vy - set Vx to Vx XOR Vy
void Chip8::op8XY3() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get Vy
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    // set Vx to Vx XOR Vy
    registers[Vx] ^= registers[Vy];
}

// 8XY4 - ADD Vx, Vy - set Vx to Vx + Vy, set VF to carry
void Chip8::op8XY4() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get Vy
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    // calculate sum
    uint16_t sum = registers[Vx] + registers[Vy];

    // if there is a carry
    if (sum > 255U) {
        // set carry
        registers[0xF] = 1;
    } else {
        // clear carry
        registers[0xF] = 0;
    }

    // set Vx to Vx + Vy
    registers[Vx] = sum & 0xFFu;
}

// 8XY5 - SUB Vx, Vy - set Vx to Vx - Vy, set VF to NOT borrow
void Chip8::op8XY5() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get Vy
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    // calculate difference
    uint16_t difference = registers[Vx] - registers[Vy];

    // if there is a borrow
    if (registers[Vx] > registers[Vy]) {
        // set borrow
        registers[0xF] = 1;
    } else {
        // clear borrow
        registers[0xF] = 0;
    }

    // set Vx to Vx - Vy
    registers[Vx] = difference & 0xFFu;
}

// 8XY6 - SHR Vx - set Vx to Vx SHR 1
void Chip8::op8XY6() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // save least significant bit in VF
    registers[0xF] = registers[Vx] & 0x01;

    // set Vx to Vx SHR 1
    registers[Vx] >>= 1;
}

// 8XY7 - SUBN Vx, Vy - set Vx to Vy - Vx, set VF to NOT borrow
void Chip8::op8XY7() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get Vy
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    // calculate difference
    uint16_t difference = registers[Vy] - registers[Vx];

    // if there is a borrow
    if (registers[Vy] > registers[Vx]) {
        // set borrow
        registers[0xF] = 1;
    } else {
        // clear borrow
        registers[0xF] = 0;
    }

    // set Vx to Vy - Vx
    registers[Vx] = difference & 0xFFu;
}

// 8XYE - SHL Vx - set Vx to Vx SHL 1
void Chip8::op8XYE() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // save most significant bit in VF
    registers[0xF] = registers[Vx] & 0x80;

    // set Vx to Vx SHL 1
    registers[Vx] <<= 1;
}

// 9XY0 - SNE Vx, Vy - skip next instruction if Vx != Vy
void Chip8::op9XY0() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get Vy
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    // check if Vx != Vy
    if (registers[Vx] != registers[Vy]) {
        // skip next instruction
        pc += 2;
    }
}

// ANNN - LD I, addr - set I to addr
void Chip8::opANNN() {
    // get address
    uint16_t address = opcode & 0x0FFF;

    // set index to address
    index = address;
}

// BNNN - JP V0, addr - set pc to V0 + addr
void Chip8::opBNNN() {
    // get address
    uint16_t address = opcode & 0x0FFF;

    // set pc to V0 + address
    pc = registers[0] + address;
}

// CXNN - RND Vx, byte - set Vx to random byte AND nn
void Chip8::opCXNN() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get NN
    uint8_t NN = opcode & 0x00FF;

    // set Vx to random byte AND NN
    registers[Vx] = getRandomByte() & NN;
}

// DXYN - DRW Vx, Vy, nibble - display n-byte sprite starting at memory location I at (Vx, Vy), set VF to collision
void Chip8::opDXYN() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get Vy
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    // get height
    uint8_t height = opcode & 0x000Fu;

    // wrap
    uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
    uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

    // set VF to collision
    registers[0xF] = 0; 

    for (unsigned int row = 0; row < height; ++row) {
        // get sprite byte
        uint8_t spriteByte = memory[index + row];

        for (unsigned int col = 0; col < 8; ++col) {
            // get sprite pixel
            uint8_t spritePixel = spriteByte & (0x80 >> col);

            // get screen pixel
            uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

            // if sprite pixel is also on
            if (spritePixel) {
                // if screen pixel is also on
                if (*screenPixel == 0xFFFFFFFF) {
                    // set collision
                    registers[0xF] = 1;
                }

                // xor screen pixel with sprite pixel
                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}

// EX9E - SKP Vx - skip next instruction if key with the value of Vx is pressed
void Chip8::opEX9E() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get key
    uint8_t key = registers[Vx];

    // check if key with the value of Vx is pressed
    if(keypad[key]) {
        // skip next instruction
        pc += 2;
    }
}

// EXA1 - SKNP Vx - skip next instruction if key with the value of Vx is not pressed
void Chip8::opEXA1() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get key
    uint8_t key = registers[Vx];

    // check if key with the value of Vx is not pressed
    if(!keypad[key]) {
        // skip next instruction
        pc += 2;
    }
}

// FX07 - LD Vx, DT - set Vx to delay timer value
void Chip8::opFX07() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // set Vx to delay timer value
    registers[Vx] = delayTimer;
}

// FX0A - LD Vx, K - wait for a key press, store the value of the key in Vx
void Chip8::opFX0A() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    bool keyPressed = false;

    // wait for a key press
    for (uint8_t key = 0; key < 16; ++key) {
        // check if key is pressed
        if (keypad[key]) {
            // set Vx to key value
            registers[Vx] = key;

            // break out of loop
            break;
        }
    }

    // check if a key was pressed
    if (!keyPressed) {
        // skip next instruction
        pc -= 2;
    }
}

// FX15 - LD DT, Vx - set delay timer to Vx
void Chip8::opFX15() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // set delay timer to Vx
    delayTimer = registers[Vx];
}

// FX18 - LD ST, Vx - set sound timer to Vx
void Chip8::opFX18() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // set sound timer to Vx
    soundTimer = registers[Vx];
}

// FX1E - ADD I, Vx - set Index to Index + Vx
void Chip8::opFX1E() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // set I to I + Vx
    index += registers[Vx];
}

// FX29 - LD F, Vx - set I to location of sprite for digit Vx
void Chip8::opFX29() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get digit
    uint8_t digit = registers[Vx];

    // set I to location of sprite for digit Vx
    index = FONTSET_START_ADDRESS + (digit * 5);
}

// FX33 - LD B, Vx - store BCD representation of Vx in memory locations I, I+1, and I+2
void Chip8::opFX33() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // get value
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
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // store registers V0 through Vx in memory starting at location I
    for (uint8_t i = 0; i <= Vx; ++i) {
        memory[index + i] = registers[i];
    }
}

// FX65 - LD Vx, [I] - read registers V0 through Vx from memory starting at location I
void Chip8::opFX65() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

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