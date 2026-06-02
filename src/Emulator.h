#ifndef __EMULATOR_H__
#define __EMULATOR_H__

#include <string>
#include <stack>
#include <cstdint>

constexpr uint16_t FontOffset = 0x50u;

class SoundManager;
class InputManager;

class Emulator
{
public:
    Emulator(InputManager* InInputManager, SoundManager* InSoundManager);
    virtual ~Emulator() = default;

    bool LoadROM(const std::string& InFile);

    void DecrementTimers();
    void ProcessInstruction();

    const bool* GetDisplay() const { return Display; }

    unsigned char GetNextOpcode() const { return (unsigned char)((unsigned char)*PC >> 4); }

    void PrintRegister() const;
    void PrintMemory() const;

private:
    void IncrementProgramCounter();

    unsigned char MemoryBuffer[4096]{0};
    unsigned char Register[16]{0};

    unsigned char DelayTimer{0};
    unsigned char SoundTimer{0};

    std::stack<unsigned char*> Stack{};

    unsigned char* PC{nullptr};  // The program counter
    unsigned char* I{nullptr};   // The Index Register

    bool Display[64 * 32]{false}; // Array of pixels

    bool UpdateVXBeforeShift = true;
    bool UseCosmacJump = true;

    InputManager* InputMgr{};
    SoundManager* SoundMgr{};
};

#endif // __EMULATOR_H__
