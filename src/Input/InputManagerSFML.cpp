#include "InputManagerSFML.h"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <iostream>

void InputManagerSFML::ProcessEvents(const sf::Event& InEvent)
{
    if (const auto* keyPressed = InEvent.getIf<sf::Event::KeyPressed>())
    {
        // Transform SFML Scancode to user Scancode
        auto code = (int)GetScancodeFromSFML(keyPressed->scancode);

        // Check if Scancode is bound to a Key
        for (int i = 0; i < 0x10; i++)
        {
            if (Scancodes[i] == code)
            {
                KeysCurrentlyPressed[i] = true;
                return;
            }
        }
    }
    else if (const auto* keyReleased = InEvent.getIf<sf::Event::KeyReleased>())
    {
        // Transform SFML Scancode to user Scancode
        auto code = (int)GetScancodeFromSFML(keyReleased->scancode);

        // Check if Scancode is bound to a Key
        for (int i = 0; i < 0x10; i++)
        {
            if (Scancodes[i] == code)
            {
                KeysCurrentlyPressed[i] = false;
                KeysReleasedThisTick.push_back(i);
                return;
            }
        }
    }
}

Scancode InputManagerSFML::GetScancodeFromSFML(sf::Keyboard::Scan InScancode) const
{
    int code = (int)InScancode;

    if (code >= (int)sf::Keyboard::Scan::A && code <= (int)sf::Keyboard::Scan::Num0)
    {
        return (Scancode)(code + 1);
    }
    if (code >= (int)sf::Keyboard::Scan::NumpadDivide && code <= (int)sf::Keyboard::Scan::NumpadPlus)
    {
        return (Scancode)(code - 25);
    }
    if (code >= (int)sf::Keyboard::Scan::Numpad1 && code <= (int)sf::Keyboard::Scan::Numpad0)
    {
        return (Scancode)(code - 27);
    }

    // TODO: Handle other scancodes
    return Scancode::Unknown;
}
