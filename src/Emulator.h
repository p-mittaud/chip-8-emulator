#ifndef __EMULATOR_H__
#define __EMULATOR_H__

#include <string>
#include <stack>

class Emulator
{
public:
    Emulator();
    virtual ~Emulator() = default;

    bool LoadROM(const std::string& InFile);

    void ProcessInstruction();

    const bool* GetDisplay() const { return Display; }

private:
    void IncrementProgramCounter();

    char MemoryBuffer[4096]{0};
    unsigned char Register[16]{0};

    std::stack<char*> Stack{};

    char* PC{nullptr};  // The program counter
    char* I{nullptr};   // The Index Register

    bool Display[64 * 32]{false}; // Array of pixels

    bool UpdateVXBeforeShift = false;
    bool UseCosmacJump = true;
};

#endif // __EMULATOR_H__
