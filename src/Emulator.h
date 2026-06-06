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
    uint32_t width{};
    uint32_t height{};
    float pixelSizeMultiplier{};
};

constexpr uint32_t DisplaySize = 128 * 64;
constexpr uint32_t WidthLowRes = 64;
constexpr uint32_t HeightLowRes = 32;
constexpr uint32_t WidthHighRes = 128;
constexpr uint32_t HeightHighRes = 64;

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
            bInLowRes ? WidthLowRes : WidthHighRes,
            bInLowRes ? HeightLowRes : HeightHighRes,
            bInLowRes ? 1.0f : 0.5f
        };
    }

    unsigned char GetNextOpcode() const { return (unsigned char)((unsigned char)*PC >> 4); }

    bool IsInLowRes() const { return bInLowRes; }

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

    bool Display[DisplaySize]{false}; // Array of pixels

    bool UpdateVXBeforeShift = true;
    bool UseCosmacJump = true;

    bool bInLowRes { true };

    InputManager* InputMgr{};
    SoundManager* SoundMgr{};

    int Type{};
};

#endif // __EMULATOR_H__
