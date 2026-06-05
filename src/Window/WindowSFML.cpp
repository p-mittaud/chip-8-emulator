#include "WindowSFML.h"

#include <SFML/Graphics.hpp>
#include "Input/InputManagerSFML.h"

WindowSFML::WindowSFML(uint32_t InWidth, uint32_t InHeight)
{
    Window = std::make_unique<sf::RenderWindow>(sf::VideoMode({InWidth, InHeight}), "CHIP-8 Emulator");

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

void WindowSFML::DrawDisplay(const bool* InDisplay, const int InWidth, const int InHeight)
{
    // TODO: Add in config
    uint32_t pixelSize = 10u;

    if (!Window)
    {
        return;
    }

    // Window::Clear
    Window->clear(sf::Color::Black);

    // Window::Render
    sf::RectangleShape rectangle({(float)pixelSize, (float)pixelSize});
    rectangle.setFillColor(sf::Color::White);

    int PixelNumber{ InWidth * InHeight };
    for (int i = 0; i < PixelNumber; i++)
    {
        if (InDisplay[i])
        {
            rectangle.setPosition({(float)(i % InWidth * pixelSize), (float)(i / InWidth * pixelSize)});
            Window->draw(rectangle);
        }
    }

    Window->display();
}
