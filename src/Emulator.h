#ifndef __EMULATOR_H__
#define __EMULATOR_H__

#include <string>
#include <stack>
#include <cstdint>

constexpr uint16_t FontOffset = 0x50u;

class SoundManager;
class InputManager;

struct EmulatorDisplay
{
    const bool* display{};
    int width{};
    int height{};
    float pixelSizeMultiplier{};
};

class Emulator
{
public:
    Emulator(InputManager* InInputManager, SoundManager* InSoundManager, int InType = 1);
    Emulator(InputManager* InInputManager, SoundManager* InSoundManager, const std::string& InROM, int InType = 1);
    virtual ~Emulator() = default;

    bool LoadROM(const std::string& InFile);

    void DecrementTimers();
    void ProcessInstruction();

    // const bool* GetDisplay() const { return Display; }
    EmulatorDisplay GetDisplay() const
    {
        return EmulatorDisplay
        {
            Display,
            bInLowRes ? 64 : 128,
            bInLowRes ? 32 : 64,
            bInLowRes ? 1.0f : 0.5f
        };
    }

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

    bool Display[128 * 64]{false}; // Array of pixels

    bool UpdateVXBeforeShift = true;
    bool UseCosmacJump = true;

    bool bInLowRes { true };

    InputManager* InputMgr{};
    SoundManager* SoundMgr{};

    int Type{};
};

#endif // __EMULATOR_H__
