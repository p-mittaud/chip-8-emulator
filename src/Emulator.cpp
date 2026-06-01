#include "Emulator.h"

#include <fstream>
#include <iostream>
#include <bitset>
#include <cstdint>

#include <cstring>

#include "Input/InputManager.h"
#include "Sound/SoundManager.h"

unsigned char EmulatorFont[80]
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

Emulator::Emulator(InputManager* InInputMgr, SoundManager* InSMgr) : InputMgr{InInputMgr}, SoundMgr{InSMgr}
{
    // Set Program Counter to the beginning of the loaded rom
    PC = &MemoryBuffer[0x200];

    // Load the font in Memory Buffer
    std::memcpy(&MemoryBuffer[FontOffset], EmulatorFont, sizeof(EmulatorFont));
}

bool Emulator::LoadROM(const std::string& InFile)
{
    std::ifstream file(InFile, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Failed to find file \"" << InFile << "\"!" << std::endl;
        return false;
    }

    file.read(&MemoryBuffer[0x200], 4096 - 0x200);
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
                for (int i = 0; i < 64*32; i++)
                {
                    Display[i] = false;
                }
            }
            else if (NNN == 0x0EE)
            {
                PC = Stack.top();
                Stack.pop();
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
                // TODO: Add configuration for SUPER-CHIP and XO-CHIP
                Register[0xF] = 0;
                    break;
                case 0x2:
                Register[X] &= Register[Y];
                // TODO: Add configuration for SUPER-CHIP and XO-CHIP
                Register[0xF] = 0;
                    break;
                case 0x3:
                Register[X] ^= Register[Y];
                // TODO: Add configuration for SUPER-CHIP and XO-CHIP
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
                    if (UpdateVXBeforeShift) // COSMAC VIP Quirk
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
                    if (UpdateVXBeforeShift) // COSMAC VIP Quirk
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
            PC = &MemoryBuffer[NNN + Register[UseCosmacJump ? 0 : X]];
            return;
        case 0xC:
            Register[X] = NN & (unsigned char)(rand() % 0x100);
            break;
        case 0xD:
        {
            int xCoord = Register[X] % 64;
            int yCoord = Register[Y] % 32;
            Register[0xF] = 0;

            for (int i = 0; i < N; i++)
            {
                if (yCoord + i >= 32)
                {
                    break;
                }

                unsigned char sprite = *(I + i);
                std::bitset<8> bitset(sprite);

                for (int pix = 0; pix < 8; pix++)
                {
                    if (xCoord + pix >= 64)
                        break;

                    if (bitset.test(7 - pix))
                    {
                        if (Display[(yCoord + i) * 64 + xCoord + pix])
                        {
                            Register[0xF] = 1;
                        }
                        Display[(yCoord + i) * 64 + xCoord + pix] = !Display[(yCoord + i) * 64 + xCoord + pix];
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
                    I = &MemoryBuffer[FontOffset + Register[X]];
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
                    // TODO: Add Increment in configuration
                    I += X + 1;
                    break;
                case 0x65:
                    for (unsigned char i = 0; i <= X; i++)
                    {
                        Register[i] = *(I + i);
                    }
                    // TODO: Add Increment in configuration
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

void Emulator::IncrementProgramCounter()
{
    PC = PC + 2;
}