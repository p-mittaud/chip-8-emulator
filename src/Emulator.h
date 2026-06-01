#ifndef __EMULATOR_H__
#define __EMULATOR_H__

#include <string>
#include <stack>
#include <cstdint>
#include <SFML/Window/Keyboard.hpp>
#include <list>

#include <SFML/Audio.hpp>

constexpr uint16_t FontOffset = 0x50u;

using namespace sf::Keyboard;

class Keypad
{
public:
    // TODO: Create an abstraction to avoid including SFML in emulator files
    // TODO: Load from a config file
    Keypad() = default;
    virtual ~Keypad() = default;

    bool IsKeyPressed(unsigned char InChar) const;

    void SetReleasedKey(int InKey);
    void ResetKeypadState();

    bool IsAnyKeyPressed() const;
    unsigned char GetKeyPressed() const;

private:
    int Scancodes[0x10]
    {
        (int)Scan::Num1, (int)Scan::Num2, (int)Scan::Num3, (int)Scan::Num4,
        (int)Scan::Q, (int)Scan::W, (int)Scan::E, (int)Scan::R,
        (int)Scan::A, (int)Scan::S, (int)Scan::D, (int)Scan::F,
        (int)Scan::Z, (int)Scan::X, (int)Scan::C, (int)Scan::V
    };

    // List of releasedScancodes this frame
    std::list<int> PressedKeys{};
};

class Emulator
{
public:
    Emulator();
    virtual ~Emulator() = default;

    bool LoadROM(const std::string& InFile);

    void DecrementTimers();
    void ProcessInstruction();

    const bool* GetDisplay() const { return Display; }

    // Keypad functions
    void SetReleasedKey(int InKey);
    void ResetKeypadState();

private:
    void IncrementProgramCounter();

    char MemoryBuffer[4096]{0};
    unsigned char Register[16]{0};

    unsigned char DelayTimer{0};
    unsigned char SoundTimer{255};

    std::stack<char*> Stack{};

    char* PC{nullptr};  // The program counter
    char* I{nullptr};   // The Index Register

    bool Display[64 * 32]{false}; // Array of pixels

    bool UpdateVXBeforeShift = false;
    bool UseCosmacJump = true;

    Keypad keypad{};

    sf::SoundBuffer soundBuffer;
    sf::Sound sound;
};

#endif // __EMULATOR_H__
