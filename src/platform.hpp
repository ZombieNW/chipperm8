#pragma once

#include <SDL2/SDL.h>
#include <cstdint>
#include <string>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

class Platform
{
public:
    Platform(char const* title, int windowWidth, int windowHeight);

    ~Platform();

    void Update(void const* buffer, int pitch);

    bool ProcessInput(uint8_t* keys);

private:
    int MapKey(SDL_Keycode sym);
    void RenderUI();

    float canvasAspect;

    SDL_Window* window{nullptr};
    SDL_Renderer* renderer{nullptr};
    SDL_Texture* texture{nullptr};
};
