cmake_minimum_required(VERSION 3.15)
project(chip-8-emulator CXX)

# Force le standard C++ moderne
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Recherche des dépendances via vcpkg
find_package(SFML COMPONENTS Graphics Audio Window System REQUIRED)

# 2. Déclaration de l'exécutable et de ses sources (chemin relatif depuis la racine)
add_executable(chip-8-emulator 
    src/main.cpp 
)

# 3. Liaison avec les bibliothèques
target_link_libraries(chip-8-emulator PRIVATE 
    SFML::Graphics 
    SFML::Audio 
    SFML::Window 
    SFML::System
)

# 4. Optionnel : Inclure le dossier src pour que les fichiers trouvent leurs .hpp facilement
target_include_directories(chip-8-emulator PRIVATE src)