#ifndef __EMULATOR_H__
#define __EMULATOR_H__

#include <string>
#include <stack>
#include <cstdint>
#include <map>

constexpr uint16_t FontOffset = 0x50u;
constexpr uint16_t HiResFontOffset = 0xA0u;

class SoundManager;
class InputManager;

struct EmulatorDisplay
{
    const unsigned char* display{};
    uint32_t width{};
    uint32_t height{};
    float pixelSizeMultiplier{};
};

constexpr uint32_t DisplaySize = 128 * 64;
constexpr uint32_t WidthLowRes = 64;
constexpr uint32_t HeightLowRes = 32;
constexpr uint32_t WidthHighRes = 128;
constexpr uint32_t HeightHighRes = 64;

struct Instruction
{
    Instruction() = default;

    Instruction(uint8_t InByte1, uint8_t InByte2)
    {
        NN = InByte2;

        // Parse the content of instruction
        Opcode = InByte1 >> 4;
        X = InByte1 & 0xF;
        Y = NN >> 4;
        N = NN & 0xF;
        NNN = (uint16_t(X) << 8) | NN;
    }

    unsigned char Opcode{}, X{}, Y{}, N{}, NN{};
    uint16_t NNN {};
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

    unsigned char GetNextOpcode() const { return (unsigned char)(MemoryBuffer[PC] >> 4); }

    bool IsInLowRes() const { return bInLowRes; }

    void PrintRegister() const;
    void PrintMemory() const;

    std::string GetSaveFileName() const;

    bool ShouldStop() const { return bShouldStop; }

private:
    void IncrementProgramCounter();

    void Draw8BitSprite(int width, int height, int xCoord, int yCoord, unsigned char N, unsigned char byteOffset, int memoryOffset);
    void Draw16BitSprite(int width, int height, int xCoord, int yCoord, unsigned char N, unsigned char byteOffset, int memoryOffset);

    typedef void (Emulator::*OpcodeFunction)();
    std::map<uint8_t, OpcodeFunction> OpcodeFunctions;

    std::map<uint8_t, OpcodeFunction> Opcode0x0Functions;
    std::map<uint8_t, OpcodeFunction> Opcode0x5Functions;
    std::map<uint8_t, OpcodeFunction> Opcode0x8Functions;
    std::map<uint8_t, OpcodeFunction> Opcode0xEFunctions;
    std::map<uint8_t, OpcodeFunction> Opcode0xFFunctions;

    void LoadOpcodes();
    void LoadOpcodes0x0();
    void LoadOpcodes0x5();
    void LoadOpcodes0x8();
    void LoadOpcodes0xE();
    void LoadOpcodes0xF();

    void Handle0x0();
    void Handle0x0E0();
    void Handle0x0EE();
    void Handle0x0FF();
    void Handle0x0FE();
    void Handle0x0FB();
    void Handle0x0FC();
    void Handle0x0CN();
    void Handle0x0DN();

    void Handle0x1();
    void Handle0x2();
    void Handle0x3();
    void Handle0x4();

    void Handle0x5();
    void Handle0x5XY0();
    void Handle0x5XY2();
    void Handle0x5XY3();

    void Handle0x6();
    void Handle0x7();
    void Handle0x9();

    void Handle0x8();
    void Handle0x8XY0();
    void Handle0x8XY1();
    void Handle0x8XY2();
    void Handle0x8XY3();
    void Handle0x8XY4();
    void Handle0x8XY5();
    void Handle0x8XY6();
    void Handle0x8XY7();
    void Handle0x8XYE();

    void Handle0xA();
    void Handle0xB();
    void Handle0xC();
    void Handle0xD();

    void Handle0xE();
    void Handle0xEX9E();
    void Handle0xEXA1();

    void Handle0xF();
    void Handle0xF000();
    void Handle0xFX01();
    void Handle0xF002();
    void Handle0xFX07();
    void Handle0xFX0A();
    void Handle0xFX15();
    void Handle0xFX18();
    void Handle0xFX1E();
    void Handle0xFX29();
    void Handle0xFX30();
    void Handle0xFX33();
    void Handle0xFX3A();
    void Handle0xFX55();
    void Handle0xFX65();
    void Handle0xFX75();
    void Handle0xFX85();

    void SkipNextInstruction();

    void HandleError();
    void HandleOpcodeError();

    // Memory and Register
    unsigned char MemoryBuffer[0x10000]{0};
    uint32_t MemoryBufferSize{0};
    unsigned char Register[16]{0};
    uint16_t PC{};  // The program counter
    uint16_t I{};   // The Index Register
    bool bIncrementPC{ true };

    unsigned char DelayTimer{0};
    unsigned char SoundTimer{0};

    std::stack<uint16_t> Stack{};

    unsigned char Display[DisplaySize]{0}; // Array of pixels
    bool bInLowRes { true };
    int SelectedDrawingPlane{1};

    int Type{};
    
    InputManager* InputMgr{};
    SoundManager* SoundMgr{};
    
    Instruction CurrentInstruction{};

    std::string CurrentROMName{};
    std::string SaveDirectory{ "../saves/" };

    bool bShouldStop{ false };
};

#endif // __EMULATOR_H__
