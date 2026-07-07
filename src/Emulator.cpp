#include "Emulator.h"

#include <fstream>
#include <iostream>
#include <cstdint>

#include <cstring>
#include <filesystem>
#include <vector>

#include <algorithm>
#include <cmath>

#include "Input/InputManager.h"
#include "Sound/SoundManager.h"

unsigned char CHIP8Font[80]
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

const uint8_t SuperChipFont[160] = {
    0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, // 0
    0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, // 1
    0x3E, 0x66, 0x06, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x7E, // 2
    0x3E, 0x66, 0x06, 0x06, 0x3E, 0x06, 0x06, 0x06, 0x66, 0x3E, // 3
    0x06, 0x0E, 0x1E, 0x36, 0x66, 0x66, 0x7F, 0x06, 0x06, 0x06, // 4
    0x7E, 0x60, 0x60, 0x60, 0x7C, 0x06, 0x06, 0x06, 0x66, 0x3E, // 5
    0x3C, 0x66, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x3C, // 6
    0x7E, 0x66, 0x06, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x30, // 7
    0x3C, 0x66, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x3C, // 8
    0x3C, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x06, 0x66, 0x3C, // 9
    0x7E, 0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x66, 0x66, // A
    0x7C, 0x66, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x7C, // B
    0x3C, 0x66, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x66, 0x3C, // C
    0x78, 0x6C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x6C, 0x78, // D
    0x7F, 0x60, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x60, 0x7F, // E
    0x7F, 0x60, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x60, 0x60  // F
};

Emulator::Emulator(InputManager* InInputMgr, SoundManager* InSMgr, int InType)
    : InputMgr{InInputMgr}, SoundMgr{InSMgr}, Type{InType}
{
    MemoryBufferSize = Type == 4 ? 0x10000 : 0x1000;

    // Set Program Counter to the beginning of the loaded rom
    PC = 0x200;

    // Load the font in Memory Buffer
    std::memcpy(MemoryBuffer, CHIP8Font, sizeof(CHIP8Font)); // Fix for binding.ch8 which doens't get the character but directly use I
    std::memcpy(&MemoryBuffer[FontOffset], CHIP8Font, sizeof(CHIP8Font));
    std::memcpy(&MemoryBuffer[HiResFontOffset], SuperChipFont, sizeof(SuperChipFont));

    LoadOpcodes();
}

Emulator::Emulator(InputManager* InInputMgr, SoundManager* InSMgr, const std::string& InROM, int InType)
    : Emulator(InInputMgr, InSMgr, InType)
{
    LoadROM(InROM);
}

bool Emulator::LoadROM(const std::string& InFile)
{
    auto lastBackslashPos = InFile.find_last_of('\\');
    auto lastSlashPos = InFile.find_last_of('/');
    size_t index = lastBackslashPos == std::string::npos ? lastSlashPos : lastBackslashPos;
    CurrentROMName = InFile.substr(index == std::string::npos ? 0 : index + 1);
    std::cout << "Name of the current ROM : \"" << CurrentROMName << "\"" << std::endl;

    std::ifstream file(InFile, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Failed to find file \"" << InFile << "\"!" << std::endl;
        return false;
    }

    file.read(reinterpret_cast<char*>(&MemoryBuffer[0x200]), MemoryBufferSize - 0x200);
    file.close();

    return true;
}

void Emulator::DecrementTimers()
{
    if (DelayTimer > 0)
    {
        DelayTimer--;
    }
    if (SoundTimer > 0)
    {
        SoundTimer--;

        if (SoundTimer == 0)
        {
            if (SoundMgr)
            {
                SoundMgr->PlayBeepSound(false);
            }
        }
    }
}

void Emulator::ProcessInstruction()
{
    bIncrementPC = true;
    CurrentInstruction = Instruction(MemoryBuffer[PC], MemoryBuffer[PC + 1]);

    if (OpcodeFunctions.count(CurrentInstruction.Opcode))
    {
        auto Function = OpcodeFunctions[CurrentInstruction.Opcode];
        (this->*Function)();
    }

    if (bIncrementPC)
    {
        IncrementProgramCounter();
    }
}

void Emulator::Draw8BitSprite(int width, int height, int xCoord, int yCoord, unsigned char N, unsigned char byteOffset, int memoryOffset)
{
    for (int i = 0; i < CurrentInstruction.N; i++)
    {
        // If XO-CHIP, we wrap
        int yCoordDesired = yCoord + i;
        if (yCoordDesired >= height)
        {
            if (Type == 4)
            {
                yCoordDesired -= height;
            }
            else
            {
                break;
            }
        }

        unsigned char sprite = MemoryBuffer[I + i + memoryOffset];

        for (int pix = 0; pix < 8; pix++)
        {
            int xCoordDesired = xCoord + pix;
            if (xCoordDesired >= width)
            {
                if (Type == 4)
                {
                    xCoordDesired -= width;
                }
                else
                {
                    break;
                }
            }
            // if (xCoord + pix >= width)
            //     break;

            if (sprite >> (7 - pix) & 1)
            {
                if (Display[(yCoordDesired) * width + xCoordDesired] & byteOffset)
                {
                    Register[0xF] = 1;
                }
                Display[(yCoordDesired) * width + xCoordDesired] ^= byteOffset;
            }
        }
    }
}

void Emulator::Draw16BitSprite(int width, int height, int xCoord, int yCoord, unsigned char N, unsigned char byteOffset, int memoryOffset)
{
    for (int i = 0; i < 16; i++)
    {
        // If XO-CHIP, we wrap
        int yCoordDesired = yCoord + i;
        if (yCoordDesired >= height)
        {
            if (Type == 2 && !bInLowRes) // SuperChip 1.1 in hires
            {
                Register[0xF] += 16 - i;
            }

            if (Type == 4)
            {
                yCoordDesired -= height;
            }
            else
            {
                break;
            }
        }

        uint16_t value = ((uint16_t)(MemoryBuffer[I + memoryOffset + i * 2]) << 8) | MemoryBuffer[I + memoryOffset + i * 2 + 1];

        bool bHadCollision{ false };

        for (int pix = 0; pix < 16; pix++)
        {
            int xCoordDesired = xCoord + pix;
            if (xCoordDesired >= width)
            {
                if (Type == 4)
                {
                    xCoordDesired -= width;
                }
                else
                {
                    break;
                }
            }

            if (value >> (15 - pix) & 1)
            {
                if (Display[(yCoordDesired) * width + xCoordDesired] & byteOffset)
                {
                    if (Type == 2 && !bInLowRes) // SuperChip 1.1 in hires
                    {
                        bHadCollision = true;
                    }
                    else
                    {
                        Register[0xF] = 1;
                    }
                }
                Display[(yCoordDesired) * width + xCoordDesired] ^= byteOffset;
            }
        }

        if (bHadCollision)
        {
            Register[0xF] += 1;
        }
    }
}

void Emulator::PrintRegister() const
{
    std::cout << "=== Printing Register ===" << std::hex << std::endl;
    for (int i = 0; i < 0x10; i++)
    {
        std::cout << (int)Register[i] << "\t";
        if ((i + 1) % 4 == 0) std::cout << std::endl;
    }
    std::cout << std::endl;
}

void Emulator::PrintMemory() const
{
    std::cout << "=== Printing Memory ===" << std::hex << std::endl;
    for (int i = 0; i < MemoryBufferSize; i++)
    {
        if (i == 0x200)
        {
            std::cout << "--- Loaded ROM ---" << std::endl;
        }

        std::cout << (uint16_t)MemoryBuffer[i] << "\t";
        if ((i + 1) % 8 == 0) std::cout << std::endl;
    }
    std::cout << std::endl;
}

std::string Emulator::GetSaveFileName() const
{
    return SaveDirectory + CurrentROMName + ".sav";
}

void Emulator::IncrementProgramCounter()
{
    PC += 2;
}

void Emulator::LoadOpcodes()
{
    OpcodeFunctions[0x0] = (this, &Emulator::Handle0x0);
    LoadOpcodes0x0();
    OpcodeFunctions[0x1] = (this, &Emulator::Handle0x1);
    OpcodeFunctions[0x2] = (this, &Emulator::Handle0x2);
    OpcodeFunctions[0x3] = (this, &Emulator::Handle0x3);
    OpcodeFunctions[0x4] = (this, &Emulator::Handle0x4);
    OpcodeFunctions[0x5] = (this, &Emulator::Handle0x5);
    LoadOpcodes0x5();
    OpcodeFunctions[0x6] = (this, &Emulator::Handle0x6);
    OpcodeFunctions[0x7] = (this, &Emulator::Handle0x7);
    OpcodeFunctions[0x8] = (this, &Emulator::Handle0x8);
    LoadOpcodes0x8();
    OpcodeFunctions[0x9] = (this, &Emulator::Handle0x9);
    OpcodeFunctions[0xA] = (this, &Emulator::Handle0xA);
    OpcodeFunctions[0xB] = (this, &Emulator::Handle0xB);
    OpcodeFunctions[0xC] = (this, &Emulator::Handle0xC);
    OpcodeFunctions[0xD] = (this, &Emulator::Handle0xD);
    OpcodeFunctions[0xE] = (this, &Emulator::Handle0xE);
    LoadOpcodes0xE();
    OpcodeFunctions[0xF] = (this, &Emulator::Handle0xF);
    LoadOpcodes0xF();
}

void Emulator::LoadOpcodes0x0()
{
    Opcode0x0Functions[0x0E0] = (this, &Emulator::Handle0x0E0);
    Opcode0x0Functions[0x0EE] = (this, &Emulator::Handle0x0EE);
    if (Type != 1)
    {
        Opcode0x0Functions[0x0FF] = (this, &Emulator::Handle0x0FF);
        Opcode0x0Functions[0x0FE] = (this, &Emulator::Handle0x0FE);
        Opcode0x0Functions[0x0FB] = (this, &Emulator::Handle0x0FB);
        Opcode0x0Functions[0x0FC] = (this, &Emulator::Handle0x0FC);
        Opcode0x0Functions[0x0C0] = (this, &Emulator::Handle0x0CN);
    }
    if (Type == 4)
    {
        Opcode0x0Functions[0x0D0] = (this, &Emulator::Handle0x0DN);
    }
}

void Emulator::LoadOpcodes0x5()
{
    Opcode0x5Functions[0x0] = (this, &Emulator::Handle0x5XY0);
    if (Type == 4)
    {
        Opcode0x5Functions[0x2] = (this, &Emulator::Handle0x5XY2);
        Opcode0x5Functions[0x3] = (this, &Emulator::Handle0x5XY3);
    }
}

void Emulator::LoadOpcodes0x8()
{
    Opcode0x8Functions[0x0] = (this, &Emulator::Handle0x8XY0);
    Opcode0x8Functions[0x1] = (this, &Emulator::Handle0x8XY1);
    Opcode0x8Functions[0x2] = (this, &Emulator::Handle0x8XY2);
    Opcode0x8Functions[0x3] = (this, &Emulator::Handle0x8XY3);
    Opcode0x8Functions[0x4] = (this, &Emulator::Handle0x8XY4);
    Opcode0x8Functions[0x5] = (this, &Emulator::Handle0x8XY5);
    Opcode0x8Functions[0x6] = (this, &Emulator::Handle0x8XY6);
    Opcode0x8Functions[0x7] = (this, &Emulator::Handle0x8XY7);
    Opcode0x8Functions[0xE] = (this, &Emulator::Handle0x8XYE);
}

void Emulator::LoadOpcodes0xE()
{
    Opcode0xEFunctions[0x9E] = &Emulator::Handle0xEX9E;
    Opcode0xEFunctions[0xA1] = &Emulator::Handle0xEXA1;
}

void Emulator::LoadOpcodes0xF()
{
    Opcode0xFFunctions[0x07] = &Emulator::Handle0xFX07;
    Opcode0xFFunctions[0x0A] = &Emulator::Handle0xFX0A;
    Opcode0xFFunctions[0x15] = &Emulator::Handle0xFX15;
    Opcode0xFFunctions[0x18] = &Emulator::Handle0xFX18;
    Opcode0xFFunctions[0x1E] = &Emulator::Handle0xFX1E;
    Opcode0xFFunctions[0x29] = &Emulator::Handle0xFX29;
    Opcode0xFFunctions[0x33] = &Emulator::Handle0xFX33;
    Opcode0xFFunctions[0x55] = &Emulator::Handle0xFX55;
    Opcode0xFFunctions[0x65] = &Emulator::Handle0xFX65;

    if (Type != 1)
    {
        Opcode0xFFunctions[0x30] = &Emulator::Handle0xFX30;
        Opcode0xFFunctions[0x75] = &Emulator::Handle0xFX75;
        Opcode0xFFunctions[0x85] = &Emulator::Handle0xFX85;
    }
    if (Type == 4)
    {
        Opcode0xFFunctions[0x00] = &Emulator::Handle0xF000;
        Opcode0xFFunctions[0x01] = &Emulator::Handle0xFX01;
        Opcode0xFFunctions[0x02] = &Emulator::Handle0xF002;
        Opcode0xFFunctions[0x3A] = &Emulator::Handle0xFX3A;
    }
}

void Emulator::Handle0x0()
{
    if (CurrentInstruction.X != 0)
    {
        std::cerr << "Failed to find opcode for " << std::hex << (int)CurrentInstruction.Opcode << (int)CurrentInstruction.X << (int)CurrentInstruction.Y << (int)CurrentInstruction.N << std::endl;
        return;
    }

    if (Opcode0x0Functions.count(CurrentInstruction.NN))
    {
        auto Function = Opcode0x0Functions[CurrentInstruction.NN];
        (this->*Function)();
    }
    else
    {
        if (Opcode0x0Functions.count(CurrentInstruction.NN & ~0b1111)) // We remove the last four bits of NNN to handle 0x00CN and 0x00DN
        {
            auto Function = Opcode0x0Functions[CurrentInstruction.NN & ~0b1111];
            (this->*Function)();
        }
        else
        {
            std::cerr << "Failed to find opcode for " << std::hex << (int)CurrentInstruction.Opcode << (int)CurrentInstruction.X << (int)CurrentInstruction.Y << (int)CurrentInstruction.N << std::endl;
        }
    }
}

void Emulator::Handle0x0E0()
{
    // Clear screen
    std::transform(Display, Display + DisplaySize, Display,
        [=](unsigned char pixel) { return pixel & ~SelectedDrawingPlane; }
    );
}

void Emulator::Handle0x0EE()
{
    PC = Stack.top();
    Stack.pop();
}

void Emulator::Handle0x0FF()
{
    bInLowRes = false;
    memset(Display, false, DisplaySize);
}

void Emulator::Handle0x0FE()
{
    bInLowRes = true;
    memset(Display, false, DisplaySize);
}

void Emulator::Handle0x0FB()
{
    // Scroll right
    int ScrollPixelNumber = (Type == 2 && bInLowRes ? 2 : 4);
    auto width = bInLowRes ? WidthLowRes : WidthHighRes;
    auto height = bInLowRes ? HeightLowRes : HeightHighRes;

    // Do a copy of the Display array
    unsigned char DisplayCopy[width * height];
    memcpy(DisplayCopy, Display, sizeof(DisplayCopy));

    for (auto i = 0u; i < height; i++)
    {
        std::transform(DisplayCopy + i * width, DisplayCopy + i * width + (width - ScrollPixelNumber), Display + i * width + ScrollPixelNumber, Display + i * width + ScrollPixelNumber,
            [=](unsigned char srcPixel, unsigned char destPixel)
            {
                return (destPixel & ~SelectedDrawingPlane) | (srcPixel & SelectedDrawingPlane);
            }
        );

        std::transform(Display + i * width, Display + i * width + ScrollPixelNumber, Display + i * width, 
            [=](unsigned char pixel)
            {
                return pixel & ~SelectedDrawingPlane;
            }
        );
    }
}

void Emulator::Handle0x0FC()
{
    // Scroll left
    int ScrollPixelNumber = (Type == 2 && bInLowRes ? 2 : 4);
    auto width = bInLowRes ? WidthLowRes : WidthHighRes;
    auto height = bInLowRes ? HeightLowRes : HeightHighRes;

    for (auto i = 0u; i < height; i++)
    {
        std::transform(Display + i * width, Display + (i + 1) * width - ScrollPixelNumber, Display + i * width + ScrollPixelNumber, Display + i * width,
            [=](unsigned char srcPixel, unsigned char destPixel)
            {
                return (srcPixel & ~SelectedDrawingPlane) | (destPixel & SelectedDrawingPlane);
            }
        );
        // Clear remaining pixels
        std::transform(Display + (i + 1) * width - ScrollPixelNumber, Display + (i + 1) * width, Display + (i + 1) * width - ScrollPixelNumber, 
            [=](unsigned char pixel)
            {
                return pixel & ~SelectedDrawingPlane;
            }
        );
    }
}

void Emulator::Handle0x0CN()
{
    if (Type == 2 && bInLowRes)
    {
        CurrentInstruction.N /= 2;
    }

    auto width = bInLowRes ? WidthLowRes : WidthHighRes;
    auto height = bInLowRes ? HeightLowRes : HeightHighRes;

    unsigned char DisplayCopy[width * height];
    memcpy(DisplayCopy, Display, sizeof(DisplayCopy));

    std::transform(DisplayCopy, DisplayCopy + width * height - CurrentInstruction.N * width, Display + CurrentInstruction.N * width, Display + CurrentInstruction.N * width,
        [=](unsigned char srcPixel, unsigned char destPixel)
        {
            return (srcPixel & SelectedDrawingPlane) | (destPixel & ~SelectedDrawingPlane);
        }
    );

    std::transform(Display, Display + CurrentInstruction.N * width, Display,
        [=](unsigned char pixel)
        {
            return pixel & ~SelectedDrawingPlane;
        }
    );
}

void Emulator::Handle0x0DN()
{
    auto width = bInLowRes ? WidthLowRes : WidthHighRes;
    auto height = bInLowRes ? HeightLowRes : HeightHighRes;

    // Scrolls pixel up
    std::transform(Display, Display + width * height - CurrentInstruction.N * width, Display + CurrentInstruction.N * width, Display, 
        [=](unsigned char srcPixel, unsigned char destPixel)
        {
            return (srcPixel & ~SelectedDrawingPlane) | (destPixel & SelectedDrawingPlane);
        }
    );
    // Clear remaining pixels
    std::transform(Display + width * height - CurrentInstruction.N * width, Display + width * height - 1, Display + width * height - CurrentInstruction.N * width, 
        [=](unsigned char pixel)
        {
            return pixel & ~SelectedDrawingPlane;
        }
    );
}

void Emulator::Handle0x1()
{
    PC = CurrentInstruction.NNN;
    bIncrementPC = false;
}

void Emulator::Handle0x2()
{
    Stack.push(PC);
    PC = CurrentInstruction.NNN;
    bIncrementPC = false;
}

void Emulator::Handle0x3()
{
    if (Register[CurrentInstruction.X] == CurrentInstruction.NN)
    {
        SkipNextInstruction();
    }
}

void Emulator::Handle0x4()
{
    if (Register[CurrentInstruction.X] != CurrentInstruction.NN)
    {
        SkipNextInstruction();
    }
}

void Emulator::Handle0x5()
{
    if (Opcode0x5Functions.count(CurrentInstruction.N))
    {
        auto Function = Opcode0x5Functions[CurrentInstruction.N];
        (this->*Function)();
    }
    else
    {
        std::cerr << "Failed to find opcode for " << std::hex << (int)CurrentInstruction.Opcode << (int)CurrentInstruction.X << (int)CurrentInstruction.Y << (int)CurrentInstruction.N << std::endl;
    }
}

void Emulator::Handle0x5XY0()
{
    if (Register[CurrentInstruction.X] == Register[CurrentInstruction.Y])
    {
        SkipNextInstruction();
    }
}

void Emulator::Handle0x5XY2()
{
    if (Type != 4)
        return;
    // Write registers CurrentInstruction.X to CurrentInstruction.Y to I. With CurrentInstruction.X > CurrentInstruction.Y or CurrentInstruction.X < CurrentInstruction.Y
    {
        int increment = 0;
        for (int v = CurrentInstruction.X; CurrentInstruction.X <= CurrentInstruction.Y ? v <= CurrentInstruction.Y : v >= CurrentInstruction.Y; CurrentInstruction.X <= CurrentInstruction.Y ? v++ : v--)
        {
            MemoryBuffer[I + increment++] = Register[v];
        }
    }
}

void Emulator::Handle0x5XY3()
{
    if (Type != 4)
        return;
    // Load Register from I to CurrentInstruction.X to CurrentInstruction.Y. With CurrentInstruction.X > CurrentInstruction.Y or CurrentInstruction.X < CurrentInstruction.Y
    {
        int increment = 0;
        for (int v = CurrentInstruction.X; CurrentInstruction.X <= CurrentInstruction.Y ? v <= CurrentInstruction.Y : v >= CurrentInstruction.Y; CurrentInstruction.X <= CurrentInstruction.Y ? v++ : v--)
        {
            Register[v] = MemoryBuffer[I + increment++];
        }
    }
}

void Emulator::Handle0x6()
{
    Register[CurrentInstruction.X] = CurrentInstruction.NN;
}

void Emulator::Handle0x7()
{
    Register[CurrentInstruction.X] += CurrentInstruction.NN;
}

void Emulator::Handle0x8()
{
    if (Opcode0x8Functions.count(CurrentInstruction.N))
    {
        auto Function = Opcode0x8Functions[CurrentInstruction.N];
        (this->*Function)();
    }
    else
    {
        std::cerr << "Failed to find opcode for " << std::hex << (int)CurrentInstruction.Opcode << (int)CurrentInstruction.X << (int)CurrentInstruction.Y << (int)CurrentInstruction.N << std::endl;
    }
}

void Emulator::Handle0x8XY0()
{
    Register[CurrentInstruction.X] = Register[CurrentInstruction.Y];
}

void Emulator::Handle0x8XY1()
{
    Register[CurrentInstruction.X] |= Register[CurrentInstruction.Y];
    if (Type == 1)
        Register[0xF] = 0;
}

void Emulator::Handle0x8XY2()
{
    Register[CurrentInstruction.X] &= Register[CurrentInstruction.Y];
    if (Type == 1)
        Register[0xF] = 0;
}

void Emulator::Handle0x8XY3()
{
    Register[CurrentInstruction.X] ^= Register[CurrentInstruction.Y];
    if (Type == 1)
        Register[0xF] = 0;
}

void Emulator::Handle0x8XY4()
{
    unsigned char overlfow = (unsigned char)(Register[CurrentInstruction.X] + Register[CurrentInstruction.Y]) < Register[CurrentInstruction.X] ? 1 : 0;
    Register[CurrentInstruction.X] += Register[CurrentInstruction.Y];
    Register[0xF] = overlfow;
}

void Emulator::Handle0x8XY5()
{
    unsigned char flag = Register[CurrentInstruction.X] >= Register[CurrentInstruction.Y];
    Register[CurrentInstruction.X] -= Register[CurrentInstruction.Y];
    Register[0xF] = flag;
}

void Emulator::Handle0x8XY6()
{
    if (Type == 1 || Type == 4) // CHIP-8 COSMAC VIP Quirk
        Register[CurrentInstruction.X] = Register[CurrentInstruction.Y];
    unsigned char flag = Register[CurrentInstruction.X] & 0x1u;
    Register[CurrentInstruction.X] = Register[CurrentInstruction.X] >> 1;
    Register[0xF] = flag;
}

void Emulator::Handle0x8XY7()
{
    unsigned char flag = Register[CurrentInstruction.Y] >= Register[CurrentInstruction.X] ? 1 : 0;
    Register[CurrentInstruction.X] = Register[CurrentInstruction.Y] - Register[CurrentInstruction.X];
    Register[0xF] = flag;
}

void Emulator::Handle0x8XYE()
{
    if (Type == 1 || Type == 4) // CHIP-8 COSMAC VIP Quirk
        Register[CurrentInstruction.X] = Register[CurrentInstruction.Y];
    unsigned char flag = Register[CurrentInstruction.X] >> 7u;
    Register[CurrentInstruction.X] = Register[CurrentInstruction.X] << 1;
    Register[0xF] = flag;
}

void Emulator::Handle0x9()
{
    if (Register[CurrentInstruction.X] != Register[CurrentInstruction.Y])
    {
        SkipNextInstruction();
    }
}

void Emulator::Handle0xA()
{
    I = CurrentInstruction.NNN;
}

void Emulator::Handle0xB()
{
    PC = CurrentInstruction.NNN + Register[Type == 1 || Type == 4 ? 0 : CurrentInstruction.X];
    bIncrementPC = false;
}

void Emulator::Handle0xC()
{
    Register[CurrentInstruction.X] = CurrentInstruction.NN & (unsigned char)(rand() % 0x100);
}

void Emulator::Handle0xD()
{
    int width = bInLowRes ? WidthLowRes : WidthHighRes;
    int height = bInLowRes ? HeightLowRes : HeightHighRes;

    int xCoord = Register[CurrentInstruction.X] % width;
    int yCoord = Register[CurrentInstruction.Y] % height;

    int BaseMemoryOffset{};

    Register[0xF] = 0;

    void (Emulator::*DrawFunc)(int, int, int, int, unsigned char, unsigned char, int);

    std::vector<unsigned char> planesToDraw;
    if (SelectedDrawingPlane & 0b01)
    {
        planesToDraw.push_back(0b01);
    }
    if (SelectedDrawingPlane & 0b10)
    {
        planesToDraw.push_back(0b10);
    }

    if (CurrentInstruction.N != 0 || (Type == 2 && CurrentInstruction.N == 0 && bInLowRes)) // SuperChip legacy only draw 8x16 in low res
    {
        if (CurrentInstruction.N == 0)
        {
            CurrentInstruction.N = 16;
        }

        DrawFunc = &Emulator::Draw8BitSprite;
        BaseMemoryOffset = CurrentInstruction.N;
    }
    else
    {
        DrawFunc = &Emulator::Draw16BitSprite;
        BaseMemoryOffset = 16 * 2;
    }

    if (planesToDraw.size())
    {
        for (auto i = 0u; i < planesToDraw.size(); i++)
        {
            (this->*DrawFunc)(width, height, xCoord, yCoord, CurrentInstruction.N, planesToDraw[i], BaseMemoryOffset * i);
        }
    }
    else
    {
        std::cerr << "Trying to draw with no planes selected!" << std::endl;
    }
}

void Emulator::Handle0xE()
{
    if (Opcode0xEFunctions.count(CurrentInstruction.NN))
    {
        auto Function = Opcode0xEFunctions[CurrentInstruction.NN];
        (this->*Function)();
    }
    else
    {
        std::cerr << "Failed to find opcode for " << std::hex << (int)CurrentInstruction.Opcode << (int)CurrentInstruction.X << (int)CurrentInstruction.Y << (int)CurrentInstruction.N << std::endl;
    }
}

void Emulator::Handle0xEX9E()
{
    if (InputMgr && InputMgr->IsKeyPressed(Register[CurrentInstruction.X]))
    {
        SkipNextInstruction();
    }
}

void Emulator::Handle0xEXA1()
{
    if (InputMgr && !InputMgr->IsKeyPressed(Register[CurrentInstruction.X]))
    {
        SkipNextInstruction();
    }
}

void Emulator::Handle0xF()
{
    if (Opcode0xFFunctions.count(CurrentInstruction.NN))
    {
        auto Function = Opcode0xFFunctions[CurrentInstruction.NN];
        (this->*Function)();
    }
    else
    {
        std::cerr << "Failed to find opcode for " << std::hex << (int)CurrentInstruction.Opcode << (int)CurrentInstruction.X << (int)CurrentInstruction.Y << (int)CurrentInstruction.N << std::endl;
    }
}

void Emulator::Handle0xF000()
{
    if (CurrentInstruction.NNN == 0)
    {
        unsigned char b1 = MemoryBuffer[PC + 2];
        unsigned char b2 = MemoryBuffer[PC + 3];
        I = (uint16_t(b1) << 8) | b2;
        IncrementProgramCounter();
    }
    else
    {
        std::cerr << "Failed to find opcode for " << std::hex << (int)CurrentInstruction.Opcode << (int)CurrentInstruction.X << (int)CurrentInstruction.Y << (int)CurrentInstruction.N << std::endl;
    }
}

void Emulator::Handle0xFX01()
{
    SelectedDrawingPlane = CurrentInstruction.X;
}

void Emulator::Handle0xF002()
{
    if (Type == 4 && CurrentInstruction.NNN == 0x002)
    {
        SoundMgr->LoadSoundArray(&MemoryBuffer[I], 0x10);
    }
    else
    {
        std::cerr << "Failed to find opcode for " << std::hex << (int)CurrentInstruction.Opcode << (int)CurrentInstruction.X << (int)CurrentInstruction.Y << (int)CurrentInstruction.N << std::endl;
    }
}

void Emulator::Handle0xFX07()
{
    Register[CurrentInstruction.X] = DelayTimer;
}

void Emulator::Handle0xFX0A()
{
    if (InputMgr && InputMgr->IsAnyKeyReleased())
    {
        Register[CurrentInstruction.X] = InputMgr->GetReleasedKey();
    }
    else
    {
        bIncrementPC = false;
    }
}

void Emulator::Handle0xFX15()
{
    DelayTimer = Register[CurrentInstruction.X];
}

void Emulator::Handle0xFX18()
{
    SoundTimer = Register[CurrentInstruction.X];
    if (SoundMgr)
    {
        SoundMgr->PlayBeepSound(SoundTimer > 0);
    }
}

void Emulator::Handle0xFX1E()
{
    bool UpdateFlags = I + Register[CurrentInstruction.X] >= MemoryBufferSize ? true : false;
    I += Register[CurrentInstruction.X];
    // Register[0xF] = UpdateFlags ? 1 : 0; // TODO: Add quirk
}

void Emulator::Handle0xFX29()
{
    I = FontOffset + Register[CurrentInstruction.X] * 5;
}

void Emulator::Handle0xFX30()
{
    I = HiResFontOffset + Register[CurrentInstruction.X] * 10;
}

void Emulator::Handle0xFX33()
{
    MemoryBuffer[I + 2] = (char)(Register[CurrentInstruction.X] % 10);
    MemoryBuffer[I + 1] = (char)((Register[CurrentInstruction.X] / 10) % 10);
    MemoryBuffer[I + 0] = (char)((Register[CurrentInstruction.X] / 100) % 10);
}

void Emulator::Handle0xFX3A()
{
    SoundMgr->SetPitch(std::powf(2.0f, (Register[CurrentInstruction.X] - 64) / 48.0f));
}

void Emulator::Handle0xFX55()
{
    for (unsigned char i = 0; i <= CurrentInstruction.X; i++)
    {
        MemoryBuffer[I + i] = Register[i];
    }
    if (Type == 1 || Type == 4)
        I += CurrentInstruction.X + 1;
}

void Emulator::Handle0xFX65()
{
    for (unsigned char i = 0; i <= CurrentInstruction.X; i++)
    {
        Register[i] = MemoryBuffer[I + i];
    }
    if (Type == 1 || Type == 4)
        I += CurrentInstruction.X + 1;
}

void Emulator::Handle0xFX75()
{
    // Create saves folder if it doesn't exists
    std::filesystem::path savePath = std::filesystem::path(GetSaveFileName()).parent_path();
    if (!savePath.empty())
    {
        std::filesystem::create_directories(savePath);
    }

    std::ofstream file(GetSaveFileName(), std::ios::out | std::ios::binary);
    file.write(reinterpret_cast<char*>(Register), CurrentInstruction.X + 1);
    file.close();
}

void Emulator::Handle0xFX85()
{
    std::ifstream file(GetSaveFileName(), std::ios::in | std::ios::binary);
    file.read(reinterpret_cast<char*>(Register), CurrentInstruction.X + 1);
    file.close();
}

void Emulator::SkipNextInstruction()
{
    if (Type == 4) // Skip 4 bytes opcodes
    {
        if (MemoryBuffer[PC + 2] == 0xF0 && MemoryBuffer[PC + 3] == 0x00)
        {
            IncrementProgramCounter();
        }
    }
    IncrementProgramCounter();
}
