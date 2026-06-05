cmake_minimum_required(VERSION 3.15)
project(chip-8-emulator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML COMPONENTS Graphics Audio Window System REQUIRED)
find_package(tomlplusplus CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(chip-8-emulator ${SOURCES})

message(${SOURCES})

target_link_libraries(chip-8-emulator PRIVATE 
    SFML::Graphics 
    SFML::Audio 
    SFML::Window 
    SFML::System
    tomlplusplus::tomlplusplus
)

target_include_directories(chip-8-emulator PRIVATE src)