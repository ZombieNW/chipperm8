#include "platform.hpp"

Platform::Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight) {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(title, 0, 0, windowWidth, windowHeight, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight);
}

Platform::~Platform() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Platform::Update(void const* buffer, int pitch) {
    SDL_UpdateTexture(texture, nullptr, buffer, pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

bool Platform::ProcessInput(uint8_t* keys) {
    bool quit = false;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP: 
            {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                    break;
                }

                int chip8Key = MapKey(event.key.keysym.sym);
                if (chip8Key != -1) {
                    // Sets to 1 on KEYDOWN, 0 on KEYUP
                    keys[chip8Key] = (event.type == SDL_KEYDOWN);
                }
            } break;
        }
    }

    return quit;
}

// helper to map keycodes to chip-8 keys
int Platform::MapKey(SDL_Keycode sym) {
    switch (sym) {
        case SDLK_x: return 0x0; case SDLK_1: return 0x1; case SDLK_2: return 0x2; case SDLK_3: return 0x3;
        case SDLK_q: return 0x4; case SDLK_w: return 0x5; case SDLK_e: return 0x6; case SDLK_a: return 0x7;
        case SDLK_s: return 0x8; case SDLK_d: return 0x9; case SDLK_z: return 0xA; case SDLK_c: return 0xB;
        case SDLK_4: return 0xC; case SDLK_r: return 0xD; case SDLK_f: return 0xE; case SDLK_v: return 0xF;
        default:     return -1;
    }
}