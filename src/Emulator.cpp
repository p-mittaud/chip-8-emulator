#include "Emulator.h"

#include <fstream>
#include <iostream>
#include <bitset>
#include <cstdint>

#include <cstring>

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
    // Set Program Counter to the beginning of the loaded rom
    PC = &MemoryBuffer[0x200];

    // Load the font in Memory Buffer
    std::memcpy(&MemoryBuffer[FontOffset], CHIP8Font, sizeof(CHIP8Font));
    std::memcpy(&MemoryBuffer[HiResFontOffset], SuperChipFont, sizeof(SuperChipFont));
}

Emulator::Emulator(InputManager* InInputMgr, SoundManager* InSMgr, const std::string& InROM, int InType)
    : InputMgr{InInputMgr}, SoundMgr{InSMgr}, Type{InType}
{
    // Set Program Counter to the beginning of the loaded rom
    PC = &MemoryBuffer[0x200];

    // Load the font in Memory Buffer
    std::memcpy(&MemoryBuffer[FontOffset], CHIP8Font, sizeof(CHIP8Font));
    std::memcpy(&MemoryBuffer[HiResFontOffset], SuperChipFont, sizeof(SuperChipFont));

    LoadROM(InROM);
}

bool Emulator::LoadROM(const std::string& InFile)
{
    std::ifstream file(InFile, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Failed to find file \"" << InFile << "\"!" << std::endl;
        return false;
    }

    file.read(reinterpret_cast<char*>(&MemoryBuffer[0x200]), 4096 - 0x200);
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
    unsigned char byte1 = (unsigned char)*PC;
    unsigned char NN = (unsigned char)*(PC + 1);

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
                memset(Display, false, DisplaySize);
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

                for (auto i = 0u; i < height; i++)
                {
                    memmove(&Display[i * width + ScrollPixelNumber], &Display[i * width], width - ScrollPixelNumber);
                    memset(&Display[i * width], false, ScrollPixelNumber);
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
                    memmove(&Display[i * width], &Display[i * width + ScrollPixelNumber], width - ScrollPixelNumber);
                    memset(&Display[(i + 1) * width - ScrollPixelNumber], false, ScrollPixelNumber);
                }
            }
            else if (X == 0x0 && Y == 0xC)
            {
                if (Type == 2 && bInLowRes)
                {
                    N /= 2;
                }

                auto width = bInLowRes ? WidthLowRes : WidthHighRes;

                memmove(&Display[N * width], Display, sizeof(Display) - N * width);
                memset(Display, false, N * width);
            }
            else
            {
                std::cerr << "Opcode " << std::hex << (int)Opcode << NNN << " not handled!" << std::endl;
            }
            break;

        case 0x1:
            // Jump to memory address
            PC = &MemoryBuffer[NNN];
            return;
        case 0x2:
            // Start subroutine
            Stack.push(PC);
            PC = &MemoryBuffer[NNN];
            // As we jump, we don't want to increment PC
            return;
        case 0x3:
            if (Register[X] == NN)
                IncrementProgramCounter();
            break;
        case 0x4:
            if (Register[X] != NN)
                IncrementProgramCounter();
            break;
        case 0x5:
            if (Register[X] == Register[Y])
                IncrementProgramCounter();
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
                    if (Type == 1) // CHIP-8 COSMAC VIP Quirk
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
                    if (Type == 1) // CHIP-8 COSMAC VIP Quirk
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
                IncrementProgramCounter();
            break;
        case 0xA:
            I = &MemoryBuffer[NNN];
            break;
        case 0xB:
            PC = &MemoryBuffer[NNN + Register[Type == 1 ? 0 : X]];
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

            if (N != 0 || (N == 0 && bInLowRes))
            {
                if (N == 0)
                {
                    N = 16;
                }

                for (int i = 0; i < N; i++)
                {
                    if (yCoord + i >= height)
                    {
                        break;
                    }

                    unsigned char sprite = *(I + i);
                    std::bitset<8> bitset(sprite);

                    for (int pix = 0; pix < 8; pix++)
                    {
                        if (xCoord + pix >= width)
                            break;

                        if (bitset.test(7 - pix))
                        {
                            if (Display[(yCoord + i) * width + xCoord + pix])
                            {
                                Register[0xF] = 1;
                            }
                            Display[(yCoord + i) * width + xCoord + pix] = !Display[(yCoord + i) * width + xCoord + pix];
                        }
                    }
                }
            }
            else
            {
                for (int i = 0; i < 16; i++)
                {
                    if (yCoord + i >= height)
                    {
                        if (Type == 2 && !bInLowRes) // SuperChip 1.1 in hires
                        {
                            Register[0xF] += 16 - i;
                        }

                        break;
                    }

                    uint16_t value = ((uint16_t)(*(I + i * 2)) << 8) | *(I + i * 2 + 1);

                    bool bHadCollision{ false };

                    for (int pix = 0; pix < 16; pix++)
                    {
                        if (xCoord + pix >= width)
                            break;

                        if (value >> (15 - pix) & 1)
                        {
                            if (Display[(yCoord + i) * width + xCoord + pix])
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
                            Display[(yCoord + i) * width + xCoord + pix] = !Display[(yCoord + i) * width + xCoord + pix];
                        }
                    }

                    if (bHadCollision)
                    {
                        Register[0xF] += 1;
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
                    IncrementProgramCounter();
                }
            }
            else if (NN == 0xA1)
            {
                if (InputMgr && !InputMgr->IsKeyPressed(Register[X]))
                {
                    IncrementProgramCounter();
                }
            }
            else
            {
                std::cerr << std::hex << "Invalid NN \"" << (int)NN << "\" for operation " << (int)Opcode << std::dec << std::endl;
            }
            break;
        case 0xF:
            switch (NN)
            {
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
                    I += Register[X];
                    break;
                case 0x29:
                    I = &MemoryBuffer[FontOffset + Register[X] * 5];
                    break;
                case 0x30:
                    I = &MemoryBuffer[HiResFontOffset + Register[X] * 10];
                    break;
                case 0x33:
                    *(I + 2) = (char)(Register[X] % 10);
                    *(I + 1) = (char)((Register[X] / 10) % 10);
                    *(I + 0) = (char)((Register[X] / 100) % 10);
                    break;
                case 0x55:
                    for (unsigned char i = 0; i <= X; i++)
                    {
                        *(I + i) = Register[i];
                    }
                    if (Type == 1)
                        I += X + 1;
                    break;
                case 0x65:
                    for (unsigned char i = 0; i <= X; i++)
                    {
                        Register[i] = *(I + i);
                    }
                    if (Type == 1)
                        I += X + 1;
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
    for (int i = 0; i < 4096; i++)
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

void Emulator::IncrementProgramCounter()
{
    PC = PC + 2;
}