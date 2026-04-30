#pragma once

#include <SDL2/SDL.h>
#include <cstdint>
#include <string>

class Platform
{
public:
    Platform(char const* title, int windowWidth, int windowHeight);
    ~Platform();
    void Update(void const* buffer, int pitch);
    bool ProcessInput(uint8_t* keys);
    void UpdateSound(uint8_t soundTimer);

    bool isPaused = false;
    int cycleDelay = 2; // default delay
    bool romLoaded = false;
    std::string currentRomPath = "";
    bool romNeedsReload = false;

private:
    int MapKey(SDL_Keycode sym);
    void RenderUI();

    float canvasAspect;
    uint32_t waveStep = 0;

    SDL_Window* window{nullptr};
    SDL_Renderer* renderer{nullptr};
    SDL_Texture* texture{nullptr};
    SDL_AudioDeviceID audioDevice;
};
