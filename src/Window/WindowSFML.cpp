#include "WindowSFML.h"

#include <SFML/Graphics.hpp>
#include "Input/InputManagerSFML.h"

#include "Emulator.h"

WindowSFML::WindowSFML(uint32_t InWidth, uint32_t InHeight, const WindowConfiguration& InConfig)
{
    WindowConfig = InConfig;

    Window = std::make_unique<sf::RenderWindow>(sf::VideoMode({InWidth * WindowConfig.PixelSize, InHeight * WindowConfig.PixelSize}), "CHIP-8 Emulator");

    InputManager = std::make_unique<InputManagerSFML>();
    InputMgrSFML = dynamic_cast<InputManagerSFML*>(InputManager.get());

    Clock = std::make_unique<sf::Clock>();
    TimePerFrame60Hz = std::make_unique<sf::Time>(sf::seconds(1.f / 60.f));
    TimeSinceLastTimerUpdate = std::make_unique<sf::Time>(sf::Time::Zero);
}

WindowSFML::~WindowSFML()
{
    
}

bool WindowSFML::IsOpen() const
{
    return Window && Window->isOpen();
}

void WindowSFML::ProcessEvents()
{
    if (!Window)
    {
        return;
    }

    while (const std::optional event = Window->pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            Window->close();
        else if (event.has_value() && InputMgrSFML)
        {
            InputMgrSFML->ProcessEvents(event.value());
        }
    }
}

void WindowSFML::UpdateTime()
{
    if (!Clock || !TimeSinceLastTimerUpdate)
        return;

    *TimeSinceLastTimerUpdate += Clock->restart();
}

bool WindowSFML::ShouldRunFrame() const
{
    return (TimePerFrame60Hz && TimeSinceLastTimerUpdate && *TimeSinceLastTimerUpdate >= *TimePerFrame60Hz);
}

void WindowSFML::UpdateTimeSinceLastFrame()
{
    if (!TimePerFrame60Hz || !TimeSinceLastTimerUpdate)
        return;

    *TimeSinceLastTimerUpdate -= *TimePerFrame60Hz;
}

void WindowSFML::DrawDisplay(EmulatorDisplay InDisplay)
{
    if (!Window)
    {
        return;
    }

    sf::Color OffColor(WindowConfig.OffColor.r, WindowConfig.OffColor.g, WindowConfig.OffColor.b);
    sf::Color OnColor(WindowConfig.OnColor.r, WindowConfig.OnColor.g, WindowConfig.OnColor.b);
    sf::Color Color1(WindowConfig.Plane1Color.r, WindowConfig.Plane1Color.g, WindowConfig.Plane1Color.b);
    sf::Color Color2(WindowConfig.Plane2Color.r, WindowConfig.Plane2Color.g, WindowConfig.Plane2Color.b);

    uint32_t PixelSize = WindowConfig.PixelSize * InDisplay.pixelSizeMultiplier;
    
    Window->clear(OffColor);

    uint32_t PixelNumber{ InDisplay.width * InDisplay.height };
    sf::RectangleShape rectangle({(float)PixelSize, (float)PixelSize});
    
    for (uint32_t i = 0; i < PixelNumber; i++)
    {
        if (InDisplay.display[i])
        {
            rectangle.setFillColor(InDisplay.display[i] == 0b11 ? OnColor : InDisplay.display[i] == 0b01 ? Color1 : Color2);
            rectangle.setPosition({(float)(i % InDisplay.width * PixelSize), (float)(i / InDisplay.width * PixelSize)});
            Window->draw(rectangle);
        }
    }

    Window->display();
}
