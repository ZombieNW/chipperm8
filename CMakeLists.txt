cmake_minimum_required(VERSION 3.10)

project(ChipperM8 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

set(SOURCES 
    src/main.cpp
    src/platform.cpp
    src/chip8.cpp
    libs/imgui/imgui.cpp
    libs/imgui/imgui_draw.cpp
    libs/imgui/imgui_tables.cpp
    libs/imgui/imgui_widgets.cpp
    libs/imgui/backends/imgui_impl_sdl2.cpp
    libs/imgui/backends/imgui_impl_sdlrenderer2.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    src 
    libs/imgui 
    libs/imgui/backends 
    ${SDL2_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})