#include <SFML/Graphics.hpp>
#include <iostream>
#include <filesystem>

#include "Emulator.h"

int main()
{
    // Get dimensions
    constexpr uint32_t width = 64u;
    constexpr uint32_t height = 32u;
    uint32_t pixelSize = 10u;

    sf::RenderWindow window(sf::VideoMode({width * pixelSize, height * pixelSize}), "TestProject");

    std::cout << "Current path is " << std::filesystem::current_path() << '\n';

    std::string romFile("../resources/IBM_Logo.ch8");

    // TODO: Add default constructor to directly load ROM
    Emulator emulator;
    emulator.LoadROM(romFile);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        emulator.ProcessInstruction();

        window.clear(sf::Color::Black);

        sf::RectangleShape rectangle({(float)pixelSize, (float)pixelSize});
        rectangle.setFillColor(sf::Color::White);

        auto Display = emulator.GetDisplay();
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