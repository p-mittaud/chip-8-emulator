#include <SFML/Graphics.hpp>
#include <iostream>
#include <filesystem>
#include <ctime>

#include "Input/InputManagerSFML.h"
#include "Sound/SoundManagerSFML.h"

#include "Emulator.h"

int main()
{
    srand( (unsigned)time(NULL) );

    // Get dimensions
    constexpr uint32_t width = 64u;
    constexpr uint32_t height = 32u;
    uint32_t pixelSize = 10u;

    sf::RenderWindow window(sf::VideoMode({width * pixelSize, height * pixelSize}), "CHIP-8 Emulator");

    std::cout << "Current path is " << std::filesystem::current_path() << '\n';

    std::string romFile("../resources/IMB_logo.ch8");

    auto soundManager = std::make_unique<SoundManagerSFML>();
    soundManager->LoadBeepSound("../resources/sounds/beep.wav");

    auto inputManager = std::make_unique<InputManagerSFML>();

    // TODO: Add default constructor to directly load ROM
    Emulator emulator(inputManager.get(), soundManager.get());
    emulator.LoadROM(romFile);

    // TODO: Add to config
    int instructionsPerSecond{700};

    sf::Clock clock;
    const sf::Time timePerFrame60Hz = sf::seconds(1.f / 60.f);
    const sf::Time timePerInstruction = sf::seconds(1.f / instructionsPerSecond);
    sf::Time timeSinceLastTimerUpdate = sf::Time::Zero;
    sf::Time timeSinceLastInstructionUpdate = sf::Time::Zero;
    int instructionsPerFrame = instructionsPerSecond / 60;

    // COSMAC VIP Quirk
    bool vBlankQuirkActive = true;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            if (event.has_value())
            {
                inputManager->ProcessEvents(event.value());
            }
        }

        // TODO: Fix this
        auto diffTime = clock.restart();
        timeSinceLastTimerUpdate += diffTime;
        while (timeSinceLastTimerUpdate >= timePerFrame60Hz)
        {
            timeSinceLastTimerUpdate -= timePerFrame60Hz;
            emulator.DecrementTimers();

            bool vBlankWait = false;
            for (int i = 0; i < instructionsPerFrame; i++)
            {
                if (vBlankQuirkActive && vBlankWait)
                {
                    break;
                }

                if (emulator.GetNextOpcode() == 0xD)
                {
                    vBlankWait = true;
                }

                emulator.ProcessInstruction();

                inputManager->ClearReleasedKeys();
            }
        }

        window.clear(sf::Color::Black);

        sf::RectangleShape rectangle({(float)pixelSize, (float)pixelSize});
        rectangle.setFillColor(sf::Color::White);

        auto Display = emulator.GetDisplay();
        for (int i = 0; i < 64*32; i++)
        {
            if (Display[i])
            {
                rectangle.setPosition({(float)(i % 64 * pixelSize), (float)(i / 64 * pixelSize)});
                window.draw(rectangle);
            }
        }

        window.display();
    }

    return 0;
}