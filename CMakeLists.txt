cmake_minimum_required(VERSION 3.10)
project(ChipperM8 LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(NFD_DIR "libs/nfd")
set(IMGUI_DIR "libs/imgui")

set(SOURCES
    src/main.cpp
    src/platform.cpp
    src/chip8.cpp
    ${NFD_DIR}/nfd_win.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

if(WIN32)
    set(EXE_ICON "resources.rc")
endif()

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${EXE_ICON})

if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE
        -static-libgcc
        -static-libstdc++
        -static
    )

    find_library(SDL2_STATIC_LIB NAMES libSDL2.a SDL2 REQUIRED)
    find_library(SDL2MAIN_LIB NAMES libSDL2main.a SDL2main REQUIRED)

    set(EXTRA_LIBS
        ${SDL2MAIN_LIB}
        ${SDL2_STATIC_LIB}
        mingw32
        m
        user32
        gdi32
        winmm
        imm32
        ole32
        oleaut32
        version
        uuid
        setupapi
        shell32
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE SDL_MAIN_HANDLED)

elseif(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
endif()

find_package(SDL2 REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    libs/imgui
    libs/imgui/backends
    ${NFD_DIR}
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${EXTRA_LIBS}
)
