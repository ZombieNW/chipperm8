#include <chrono>
#include <iostream>
#include <string>
#include "platform.hpp"
#include "chip8.hpp"

int main(int argc, char** argv) {
    Platform platform("Chipper-M8", 1280, 720);
    Chip8 chip8;

    // cli args
    if (argc >= 2) {
        platform.currentRomPath = argv[1];
        if (chip8.loadROM(platform.currentRomPath.c_str())) {
            platform.romLoaded = true;
        }
    }
    if (argc >= 3) {
        platform.cycleDelay = std::stoi(argv[2]);
    }

    int videoPitch = sizeof(chip8.video[0]) * VIDEO_WIDTH;
    auto lastCycleTime = std::chrono::high_resolution_clock::now();
    bool quit = false;

    while (!quit) {
        quit = platform.ProcessInput(chip8.keypad);

        // check if ui asked to reload
        if (platform.romNeedsReload) {
            chip8.reset(); 

            if (chip8.loadROM(platform.currentRomPath.c_str())) {
                platform.romLoaded = true;
            } else {
                platform.romLoaded = false;
            }
            platform.romNeedsReload = false;
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::milli>(currentTime - lastCycleTime).count();

        if (platform.romLoaded && !platform.isPaused) {
            if (dt > platform.cycleDelay) {
                lastCycleTime = currentTime;
                chip8.cycle();

                platform.UpdateSound(chip8.soundTimer);
            }
        }

        platform.Update(chip8.video, videoPitch);
    }

    return 0;
}