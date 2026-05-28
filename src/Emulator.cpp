#include "Emulator.h"

#include <fstream>
#include <iostream>
#include <bitset>
#include <cstdint>

Emulator::Emulator()
{
    // Set Program Counter to the beginning of the loaded rom
    PC = &MemoryBuffer[0x200];
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
            break;

        case 0x1:
            // Jump to memory address
            PC = &MemoryBuffer[NNN];
            return;
        case 0x6:
            Register[X] = NN;
            break;
        case 0x7:
            Register[X] += NN;
            break;
        case 0xA:
            I = &MemoryBuffer[NNN];
            break;
        // D
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