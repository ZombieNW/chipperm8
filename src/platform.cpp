#include "platform.hpp"
#include "chip8.hpp"
#include "nfd.hpp"

Platform::Platform(char const* title, int windowWidth, int windowHeight) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO);

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, VIDEO_WIDTH, VIDEO_HEIGHT);

    canvasAspect = (float)VIDEO_WIDTH / (float)VIDEO_HEIGHT;

    // setup imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // audio
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 2048;

    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    SDL_PauseAudioDevice(audioDevice, 0);
}

Platform::~Platform() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    if (audioDevice) {
        SDL_CloseAudioDevice(audioDevice);
    }
    SDL_Quit();
}

void Platform::Update(void const* buffer, int pitch) {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    RenderUI();

    // update emulator texture
    SDL_UpdateTexture(texture, nullptr, buffer, pitch);

    // setup background window
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos);
    ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize);

    // remove padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::Begin("EMU", nullptr, 
    ImGuiWindowFlags_NoDecoration | 
    ImGuiWindowFlags_NoMove | 
    ImGuiWindowFlags_NoBringToFrontOnFocus | 
    ImGuiWindowFlags_NoInputs);

    ImVec2 winSize = ImGui::GetContentRegionAvail();
    float w = winSize.x;
    float h = winSize.y;

    // keep aspect
    if (w / h > canvasAspect) {
        w = h * canvasAspect;
    } else {
        h = w / canvasAspect;
    }

    // center offset
    float offsetX = (winSize.x - w) * 0.5f;
    float offsetY = (winSize.y - h) * 0.5f;

    ImGui::SetCursorPos(ImVec2(offsetX, offsetY));

    // draw sdl canvas as imgui image
    if (!romLoaded) {
        const char* text = "No ROM loaded. Go to File > Open ROM to start.";
        ImVec2 textSize = ImGui::CalcTextSize(text);

        float textX = offsetX + (w - textSize.x) * 0.5f;
        float textY = offsetY + (h - textSize.y) * 0.5f;

        ImGui::SetCursorPos(ImVec2(textX, textY));
        ImGui::Text("%s", text);
    } else {
        ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2(w, h));
    }
    ImGui::End();
    ImGui::PopStyleVar();

    // render
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    
    SDL_RenderPresent(renderer);
}

void Platform::RenderUI() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open ROM...", "Ctrl+O")) {
                NFD::UniquePath outPath;
                nfdfilteritem_t filterItem[1] = {{"Chip-8 ROM", "ch8,bin,rom"}};

                nfdresult_t result = NFD::OpenDialog(outPath, filterItem, 1);

                if (result == NFD_OKAY) {
                    this->currentRomPath = outPath.get();
                    this->romNeedsReload = true;
                } else {
                    std::cerr << "Error: " << NFD::GetError() << std::endl;
                } 
            }

            if (ImGui::MenuItem("Quit", "esc")) {
                SDL_Event quitEvent;
                quitEvent.type = SDL_QUIT;
                SDL_PushEvent(&quitEvent);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Emulation")) {
            if (ImGui::MenuItem("Pause", "P", isPaused)) {
                isPaused = !isPaused;
            }

            if (ImGui::MenuItem("Reset", "R", isPaused)) {
                this->romNeedsReload = true;
            }

            if (ImGui::BeginMenu("Cycle Delay (ms)")) {
                for (int i : {0, 1, 2, 5, 10, 16, 33}) {
                    std::string label = std::to_string(i) + " ms";
                    if (ImGui::MenuItem(label.c_str(), nullptr, cycleDelay == i)) {
                        cycleDelay = i;
                    }
                }
                ImGui::EndMenu();
            }
            
            ImGui::EndMenu();
        }

        ImGui::Separator();
        if (romLoaded) {
            ImGui::TextDisabled("Running: %s", currentRomPath.c_str());
        } else {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "No ROM Loaded");
        }

        ImGui::EndMainMenuBar();
    }
}

void Platform::UpdateSound(uint8_t soundTimer) {
    if (soundTimer > 0) {
        int samplesToQueue = 44100 / 60; 
        std::vector<int16_t> samples(samplesToQueue);

        for (int i = 0; i < samplesToQueue; ++i) {
            // 440Hz square wave
            samples[i] = ((waveStep++ / (44100 / 440 / 2)) % 2) ? 3000 : -3000;
        }
        
        // only queue if the buffer is getting low to avoid latency
        if (SDL_GetQueuedAudioSize(audioDevice) < samplesToQueue * sizeof(int16_t) * 2) {
            SDL_QueueAudio(audioDevice, samples.data(), samples.size() * sizeof(int16_t));
        }
    } else {
        SDL_ClearQueuedAudio(audioDevice);
        waveStep = 0;
    }
}

bool Platform::ProcessInput(uint8_t* keys) {
    bool quit = false;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_QUIT) quit = true;
        
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            bool isDown = (event.type == SDL_KEYDOWN);
            
            // always quit on escape
            if (isDown && event.key.keysym.sym == SDLK_ESCAPE) quit = true;
            // pause
            if (isDown && event.key.keysym.sym == SDLK_p) {
                this->isPaused = !this->isPaused;
            }
            if (isDown && event.key.keysym.sym == SDLK_r) {
                this->romNeedsReload = true;
            }
            if ((SDL_GetModState() & KMOD_CTRL) && event.key.keysym.sym == SDLK_o) {
                NFD::UniquePath outPath;
                nfdfilteritem_t filterItem[1] = {{"Chip-8 ROM", "ch8,bin"}};

                nfdresult_t result = NFD::OpenDialog(outPath, filterItem, 1);

                if (result == NFD_OKAY) {
                    this->currentRomPath = outPath.get();
                    this->romNeedsReload = true;
                } else {
                    std::cerr << "Error: " << NFD::GetError() << std::endl;
                } 
            }
            


            // only send keys to emulator if imgui isn't using them
            if (!ImGui::GetIO().WantTextInput) {
                int chip8Key = MapKey(event.key.keysym.sym);
                if (chip8Key != -1) {
                    keys[chip8Key] = isDown ? 1 : 0;
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