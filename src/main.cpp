#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>

#include <filesystem>

#include <bitset>

int main()
{
    // Get dimensions
    constexpr uint32_t width = 64u;
    constexpr uint32_t height = 32u;
    uint32_t pixelSize = 10u;

    sf::RenderWindow window(sf::VideoMode({width * pixelSize, height * pixelSize}), "TestProject");

    // Read instructions from program and store them in the stack
    char memoryBuffer[4096]{0};

    std::cout << "Current path is " << std::filesystem::current_path() << '\n';

    std::string romFile("../resources/IBM_Logo.ch8");
    std::ifstream file(romFile, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Failed to find file!" << std::endl;
    }

    file.read(&memoryBuffer[0x200], 4096 - 0x200);
    file.close();

    char* ProgramCounter{&memoryBuffer[0x200]};
    char* IndexRegister{nullptr};
    unsigned char Register[16]{0};

    bool Display[64*32]{false};

    while (window.isOpen()) {


        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

        }

        // TODO: Create object: Core emulator function
        {
            unsigned char byte1 = (unsigned char)*ProgramCounter;
            unsigned char NN = (unsigned char)*(ProgramCounter + 1);

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
                    ProgramCounter = &memoryBuffer[NNN];
                    continue;
                case 0x6:
                    Register[X] = NN;
                    break;
                case 0x7:
                    Register[X] += NN;
                    break;
                case 0xA:
                    IndexRegister = &memoryBuffer[NNN];
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

                        unsigned char sprite = *(IndexRegister + i);
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

            std::cout << std::hex << (int)Opcode << " - " << (int)X << std::endl;

            // Update program counter
            ProgramCounter = ProgramCounter + 2;
        }

        window.clear(sf::Color::Black);

        sf::RectangleShape rectangle({(float)pixelSize, (float)pixelSize});
        rectangle.setFillColor(sf::Color::White);
        float tmp{0.0f};
        for (int i = 0; i < 64*32; i++)
        {
            if (Display[i])
            {
                // rectangle.setPosition({(float)(i / 32 * pixelSize), (float)(i % 64 * pixelSize)});
                rectangle.setPosition({(float)(i % 64 * pixelSize), (float)(i / 64 * pixelSize)});
                window.draw(rectangle);
            }
        }

        window.display();
    }

    return 0;
}