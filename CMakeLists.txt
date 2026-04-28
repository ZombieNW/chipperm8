cmake_minimum_required(VERSION 3.10)

project(ChipperM8 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

set(NFD_DIR "libs/nfd")
set(IMGUI_DIR "libs/imgui")

set(SOURCES 
    src/main.cpp
    src/platform.cpp
    src/chip8.cpp
    #nfd
    ${NFD_DIR}/nfd_win.cpp
    #imgui
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    src 
    libs/imgui 
    libs/imgui/backends 
    ${NFD_DIR} 
    ${SDL2_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${SDL2_LIBRARIES} 

)