#include "chip8.hpp"

// constants
const unsigned int START_ADDRESS = 0x200; // first 512 bytes reserved for interpreter
const unsigned int FONTSET_SIZE = 80; // size of all 16 characters (0-F) in bytes
const unsigned int FONTSET_START_ADDRESS = 0x50; // location of fontset

class Chip8
{
    public:
        uint8_t registers[16]{};
        uint8_t memory[4096]{};
        uint16_t index{};
        uint16_t pc{};
        uint16_t stack[16]{};
        uint8_t sp{};
        uint8_t delayTimer{};
        uint8_t soundTimer{};
        uint8_t keypad[16]{};
        uint32_t video[64 * 32]{};
        uint16_t opcode;

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

    std::default_random_engine randGen;
	std::uniform_int_distribution<uint8_t> randByte;

    // initialization
    void Chip8::initialize() {
        // program counter starts at 0x200
        pc = START_ADDRESS;

        // load fonts into memory
        for (int i = 0; i < FONTSET_SIZE; ++i) {
            memory[FONTSET_START_ADDRESS + i] = fontset[i];
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

    // random number generator sets randGen to number randByte to byte
    Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count()), randByte(0, 255) {}
};