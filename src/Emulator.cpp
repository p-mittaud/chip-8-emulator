#include "Emulator.h"

#include <fstream>
#include <iostream>
#include <bitset>
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
    std::memcpy(&MemoryBuffer[FontOffset], CHIP8Font, sizeof(CHIP8Font));
    std::memcpy(&MemoryBuffer[HiResFontOffset], SuperChipFont, sizeof(SuperChipFont));
}

Emulator::Emulator(InputManager* InInputMgr, SoundManager* InSMgr, const std::string& InROM, int InType)
    : InputMgr{InInputMgr}, SoundMgr{InSMgr}, Type{InType}
{
    MemoryBufferSize = Type == 4 ? 0x10000 : 0x1000;

    // Set Program Counter to the beginning of the loaded rom
    PC = 0x200;

    // Load the font in Memory Buffer
    std::memcpy(&MemoryBuffer[FontOffset], CHIP8Font, sizeof(CHIP8Font));
    std::memcpy(&MemoryBuffer[HiResFontOffset], SuperChipFont, sizeof(SuperChipFont));

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
    // Could create struct Instruction

    // Read the two bytes of the instruction
    unsigned char byte1 = MemoryBuffer[PC];
    unsigned char NN = MemoryBuffer[PC + 1];

    // Parse the content of instruction
    unsigned char Opcode = byte1 >> 4;
    unsigned char X = byte1 & 0xF;
    unsigned char Y = NN >> 4;
    unsigned char N = NN & 0xF;
    uint16_t NNN = (uint16_t(X) << 8) | NN;

    switch (Opcode)
    {
        case 0x0:
            if (NNN == 0xE0)
            {
                // Clear screen
                std::transform(Display, Display + DisplaySize, Display,
                    [=](unsigned char pixel) { return pixel & ~SelectedDrawingPlane; }
                );
            }
            else if (NNN == 0x0EE)
            {
                PC = Stack.top();
                Stack.pop();
            }
            else if (NNN == 0x0FF)
            {
                bInLowRes = false;
                memset(Display, false, DisplaySize);
            }
            else if (NNN == 0x0FE)
            {
                bInLowRes = true;
                memset(Display, false, DisplaySize);
            }
            else if (NNN == 0x0FB)
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
            else if (NNN == 0x0FC)
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
            else if (X == 0x0 && Y == 0xC) // Scroll Down
            {
                if (Type == 2 && bInLowRes)
                {
                    N /= 2;
                }

                auto width = bInLowRes ? WidthLowRes : WidthHighRes;
                auto height = bInLowRes ? HeightLowRes : HeightHighRes;

                unsigned char DisplayCopy[width * height];
                memcpy(DisplayCopy, Display, sizeof(DisplayCopy));

                std::transform(DisplayCopy, DisplayCopy + width * height - N * width, Display + N * width, Display + N * width,
                    [=](unsigned char srcPixel, unsigned char destPixel)
                    {
                        return (srcPixel & SelectedDrawingPlane) | (destPixel & ~SelectedDrawingPlane);
                    }
                );

                std::transform(Display, Display + N * width, Display,
                    [=](unsigned char pixel)
                    {
                        return pixel & ~SelectedDrawingPlane;
                    }
                );
            }
            else if (X == 0x0 && Y == 0xD && Type == 4) // Scroll Up
            {
                auto width = bInLowRes ? WidthLowRes : WidthHighRes;
                auto height = bInLowRes ? HeightLowRes : HeightHighRes;

                // Scrolls pixel up
                std::transform(Display, Display + width * height - N * width, Display + N * width, Display, 
                    [=](unsigned char srcPixel, unsigned char destPixel)
                    {
                        return (srcPixel & ~SelectedDrawingPlane) | (destPixel & SelectedDrawingPlane);
                    }
                );
                // Clear remaining pixels
                std::transform(Display + width * height - N * width, Display + width * height - 1, Display + width * height - N * width, 
                    [=](unsigned char pixel)
                    {
                        return pixel & ~SelectedDrawingPlane;
                    }
                );
            }
            else
            {
                std::cerr << "Opcode " << std::hex << (int)Opcode << NNN << " not handled!" << std::endl;
            }
            break;

        case 0x1:
            // Jump to memory address
            PC = NNN;
            return;
        case 0x2:
            // Start subroutine
            Stack.push(PC);
            PC = NNN;
            // As we jump, we don't want to increment PC
            return;
        case 0x3:
            if (Register[X] == NN)
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
            break;
        case 0x4:
            if (Register[X] != NN)
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
            break;
        case 0x5:
            switch (N)
            {
                case 0:
                {
                if (Register[X] == Register[Y])
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
                break;
                }
                case 2:
                    if (Type != 4)
                        break;
                    // Write registers X to Y to I. With X > Y or X < Y
                    {
                        int increment = 0;
                        for (int v = X; X <= Y ? v < Y : v > Y; X <= Y ? v++ : v--)
                        {
                            MemoryBuffer[I + increment++] = Register[v];
                        }
                    }
                    break;
                case 3:
                    if (Type != 4)
                        break;
                    // Load Register from I to X to Y. With X > Y or X < Y
                    {
                        int increment = 0;
                        for (int v = X; X <= Y ? v < Y : v > Y; X <= Y ? v++ : v--)
                        {
                            Register[v] = MemoryBuffer[I + increment++];
                        }
                    }
                    break;
                default:
                    std::cerr << "Operation 0x5XY" << N << " is not handled!" << std::endl;
                    break;
            }
            break;
        case 0x6:
            Register[X] = NN;
            break;
        case 0x7:
            Register[X] += NN;
            break;
        case 0x8:
            switch (N)
            {
                case 0x0:
                Register[X] = Register[Y];
                    break;
                case 0x1:
                Register[X] |= Register[Y];
                if (Type == 1)
                    Register[0xF] = 0;
                    break;
                case 0x2:
                Register[X] &= Register[Y];
                if (Type == 1)
                    Register[0xF] = 0;
                    break;
                case 0x3:
                Register[X] ^= Register[Y];
                if (Type == 1)
                    Register[0xF] = 0;
                    break;
                case 0x4:
                {
                    unsigned char overlfow = (unsigned char)(Register[X] + Register[Y]) < Register[X] ? 1 : 0;
                    Register[X] += Register[Y];
                    Register[0xF] = overlfow;
                }
                    break;
                case 0x5:
                {
                    unsigned char flag = Register[X] >= Register[Y];
                    Register[X] -= Register[Y];
                    Register[0xF] = flag;
                }
                    break;
                case 0x6:
                {
                    if (Type == 1 || Type == 4) // CHIP-8 COSMAC VIP Quirk
                        Register[X] = Register[Y];
                    unsigned char flag = Register[X] & 0x1u;
                    Register[X] = Register[X] >> 1;
                    Register[0xF] = flag;
                }
                    break;
                case 0x7:
                {
                    unsigned char flag = Register[Y] >= Register[X] ? 1 : 0;
                    Register[X] = Register[Y] - Register[X];
                    Register[0xF] = flag;
                }
                    break;
                case 0xE:
                {
                    if (Type == 1 || Type == 4) // CHIP-8 COSMAC VIP Quirk
                        Register[X] = Register[Y];
                    unsigned char flag = Register[X] >> 7u;
                    Register[X] = Register[X] << 1;
                    Register[0xF] = flag;
                }
                    break;
                default:
                    std::cerr << "Case " << N << " not handled!" << std::endl;
            }
            break;
        case 0x9:
            if (Register[X] != Register[Y])
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
            break;
        case 0xA:
            I = NNN;
            break;
        case 0xB:
            PC = NNN + Register[Type == 1 || Type == 4 ? 0 : X];
            return;
        case 0xC:
            Register[X] = NN & (unsigned char)(rand() % 0x100);
            break;
        case 0xD:
        {
            int width = bInLowRes ? WidthLowRes : WidthHighRes;
            int height = bInLowRes ? HeightLowRes : HeightHighRes;

            int xCoord = Register[X] % width;
            int yCoord = Register[Y] % height;
            Register[0xF] = 0;

            if (N != 0 || (Type == 2 && N == 0 && bInLowRes)) // SuperChip legacy only draw 8x16 in low res
            {
                if (N == 0)
                {
                    N = 16;
                }

                std::vector<unsigned char> planesToDraw;
                if (SelectedDrawingPlane & 0b01)
                {
                    planesToDraw.push_back(0b01);
                }
                if (SelectedDrawingPlane & 0b10)
                {
                    planesToDraw.push_back(0b10);
                }

                if (planesToDraw.size())
                {
                    for (auto i = 0u; i < planesToDraw.size(); i++)
                    {
                        Draw8BitSprite(width, height, xCoord, yCoord, N, planesToDraw[i], N * i);
                    }
                }
            }
            else
            {
                std::vector<unsigned char> planesToDraw;
                if (SelectedDrawingPlane & 0b01)
                {
                    planesToDraw.push_back(0b01);
                }
                if (SelectedDrawingPlane & 0b10)
                {
                    planesToDraw.push_back(0b10);
                }

                if (planesToDraw.size())
                {
                    for (auto i = 0u; i < planesToDraw.size(); i++)
                    {
                         Draw16BitSprite(width, height, xCoord, yCoord, N, planesToDraw[i], 16 * 2 * i);
                    }
                }
            }
        }
            break;
        case 0xE:
            if (NN == 0x9E)
            {
                if (InputMgr && InputMgr->IsKeyPressed(Register[X]))
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
            }
            else if (NN == 0xA1)
            {
                if (InputMgr && !InputMgr->IsKeyPressed(Register[X]))
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
            }
            else
            {
                std::cerr << std::hex << "Invalid NN \"" << (int)NN << "\" for operation " << (int)Opcode << std::dec << std::endl;
            }
            break;
        case 0xF:
            if (Type == 4 && NNN == 0)
            {
                unsigned char b1 = MemoryBuffer[PC + 2];
                unsigned char b2 = MemoryBuffer[PC + 3];
                I = (uint16_t(b1) << 8) | b2;
                IncrementProgramCounter();
                break;
            }
            if (Type == 4 && NNN == 0x002)
            {
                SoundMgr->LoadSoundArray(&MemoryBuffer[I], 0x10);
                break;
            }
            switch (NN)
            {
                case 0x01:
                    if (Type == 4)
                    {
                        SelectedDrawingPlane = X;
                    }
                    else
                        std::cerr << std::hex << (int)NN << " is not handled!" << std::endl;
                    break;
                case 0x07:
                    Register[X] = DelayTimer;
                    break;
                case 0x15:
                    DelayTimer = Register[X];
                    break;
                case 0x18:
                    SoundTimer = Register[X];
                    if (SoundMgr)
                    {
                        SoundMgr->PlayBeepSound(SoundTimer > 0);
                    }
                    break;
                case 0x1E:
                    {
                        bool UpdateFlags = I + Register[X] >= MemoryBufferSize ? true : false;
                        I += Register[X];
                        Register[0xF] = UpdateFlags ? 1 : 0;
                    }
                    break;
                case 0x29:
                    I = FontOffset + Register[X] * 5;
                    break;
                case 0x30:
                    I = HiResFontOffset + Register[X] * 10;
                    break;
                case 0x33:
                    MemoryBuffer[I + 2] = (char)(Register[X] % 10);
                    MemoryBuffer[I + 1] = (char)((Register[X] / 10) % 10);
                    MemoryBuffer[I + 0] = (char)((Register[X] / 100) % 10);
                    break;
                case 0x3A:
                    if (Type != 4)
                    {
                        std::cerr << std::hex << (int)NN << " is not handled!" << std::endl;
                        break;
                    }
                    {
                        SoundMgr->SetPitch(std::powf(2.0f, (Register[X] - 64) / 48.0f));
                    }
                    break;
                case 0x55:
                    for (unsigned char i = 0; i <= X; i++)
                    {
                        MemoryBuffer[I + i] = Register[i];
                    }
                    if (Type == 1 || Type == 4)
                        I += X + 1;
                    break;
                case 0x65:
                    for (unsigned char i = 0; i <= X; i++)
                    {
                        Register[i] = MemoryBuffer[I + i];
                    }
                    if (Type == 1 || Type == 4)
                        I += X + 1;
                    break;

                case 0x75:
                    {
                        // Create saves folder if it doesn't exists
                        std::filesystem::path savePath = std::filesystem::path(GetSaveFileName()).parent_path();
                        if (!savePath.empty())
                        {
                            std::filesystem::create_directories(savePath);
                        }

                        std::ofstream file(GetSaveFileName(), std::ios::out | std::ios::binary);
                        file.write(reinterpret_cast<char*>(Register), X + 1);
                        file.close();
                    }
                    break;
                case 0x85:
                    {
                        std::ifstream file(GetSaveFileName(), std::ios::in | std::ios::binary);
                        file.read(reinterpret_cast<char*>(Register), X + 1);
                        file.close();
                    }
                    break;

                case 0x0A:
                    if (InputMgr && InputMgr->IsAnyKeyReleased())
                    {
                        Register[X] = InputMgr->GetReleasedKey();
                    }
                    else
                    {
                        return;
                    }
                    break;
                default:
                    std::cerr << std::hex << (int)NN << " is not handled!" << std::endl;
            }
            break;
        default:
            std::cerr << std::hex << "Opcode " << (int)Opcode << " not handled!" << std::dec << std::endl;
            break;
    }

    IncrementProgramCounter();
}

void Emulator::Draw8BitSprite(int width, int height, int xCoord, int yCoord, unsigned char N, unsigned char byteOffset, int memoryOffset)
{
    for (int i = 0; i < N; i++)
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
        std::bitset<8> bitset(sprite);

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

            if (bitset.test(7 - pix))
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