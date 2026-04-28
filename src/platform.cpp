#include "platform.hpp"

Platform::Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight) : texWidth(textureWidth), texHeight(textureHeight) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight);

    canvasAspect = (float)textureWidth / (float)textureHeight;

    // setup imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
}

Platform::~Platform() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Platform::Update(void const* buffer, int pitch) {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    RenderUI();

    // update emulator texture
    SDL_UpdateTexture(texture, nullptr, buffer, pitch);

    // render sdl canvas
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // canvas logic
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos);
    ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize);
    ImGui::Begin("ROM", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImVec2 winSize = ImGui::GetContentRegionAvail();
    float w = winSize.x;
    float h = winSize.y;

    if (w / h > canvasAspect) w = h * canvasAspect;
    else h = w / canvasAspect;

    ImGui::SetCursorPosX((winSize.x - w) * 0.5f);
    ImGui::SetCursorPosY((winSize.y - h) * 0.5f);

    // draw sdl canvas as imgui image
    ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2(w, h));
    ImGui::End();

    // render
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
}

void Platform::RenderUI() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open ROM...")) {
                // TODO
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

bool Platform::ProcessInput(uint8_t* keys) {
    bool quit = false;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_QUIT) quit = true;
        
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            // only send keys to emulator if imgui isn't using them
            if (!ImGui::GetIO().WantCaptureKeyboard) {
                if (event.key.keysym.sym == SDLK_ESCAPE) quit = true;
                int chip8Key = MapKey(event.key.keysym.sym);
                if (chip8Key != -1) {
                    keys[chip8Key] = (event.type == SDL_KEYDOWN);
                }
            }
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