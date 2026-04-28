#include <chrono>
#include <iostream>
#include <string>
#include "platform.hpp"
#include "chip8.hpp"

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
		std::exit(EXIT_FAILURE);
    }

    int videoScale = std::stoi(argv[1]);
	int cycleDelay = std::stoi(argv[2]);
	char const* romFilename = argv[3];

    Platform platform("CHIP-8", 640 * videoScale, 320 * videoScale, 64 * videoScale, 32 * videoScale);
    Chip8 chip8;

    chip8.loadROM(romFilename);

    int videoPitch = sizeof(chip8.video[0]) * VIDEO_WIDTH;

    auto lastCycleTime = std::chrono::high_resolution_clock::now();
    bool quit = false;

    while (!quit) {
        quit = platform.ProcessInput(chip8.keypad);

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::milli>(currentTime - lastCycleTime).count();

        if (dt > cycleDelay) {
            lastCycleTime = currentTime;
            chip8.cycle();
        }

        platform.Update(chip8.video, videoPitch);
    }

    return 0;
}