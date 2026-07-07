# CHIP-8 Emulator

A CHIP-8, SUPER-CHIP and XO-CHIP emulator written in C++.

![A white text on a black background says PM's CHIP-8 emulator.](/resources/screenshots/default.png)

This project may be used to run and test differents ROMs of differents emulators.\
If you test from banks ROMs, I suggest you to try to test the latest version of the emulator (XO-CHIP) as many new ROMs are built with a language which depends of the latest versions of the emulator.

## How to test

You can just execute the default executable and a default ROM should load if everything is okay.\
If an error occurs (invalid ROM or invalid opcode in code), the program will just stop to execute instruction but the window won't close until you request it.

### Configuration
`config.toml` is one of the main file of the project. It allows you to choose the emulator type, update frequency and ROMs to play.

### Build from source

You can build the project by cloning it. 
You will need to have:
  - MinGW C++ compiler
  - CMake
  - vcpkg

When the project is built, you just need to run the executable which will start a default ROM.

### Download the release

Download the zip from the page. Execute the `CHIP-8 Emulator.bat` file which is juste a shortcut to the application.
Currently, there is only a binary version for windows.

## References
The sites which helped me through my journey:
- Guide to making a CHIP-8 emulator - [here](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)
- Timendus CHIP-8 test suite - [here](https://github.com/Timendus/chip8-test-suite)
- CHIP-8 Variant Opcode Table - [here](https://chip8.gulrak.net)
- CHIP-8 Archive - [here](https://johnearnest.github.io/chip8Archive/)

## Maybe one day
Things that could be done to improve this project:
- being able to change the type of the emulator at runtime
- being able to load new ROMS at runtime
- adding an UI (like Dear ImGui) to have more control about emulator (pausing, dumping memory and registers)
- create new ROMs which will work on SUPER-CHIP but not on XO-CHIP or a new test suite
