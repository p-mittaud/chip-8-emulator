#ifndef __INPUT_MANAGER_SFML_H__
#define __INPUT_MANAGER_SFML_H__

#include "InputManager.h"

namespace sf
{
    class Event;

    namespace Keyboard
    {
        enum class Scan;
    }
}

class InputManagerSFML : public InputManager
{
public:
    InputManagerSFML() = default;
    virtual ~InputManagerSFML() = default;

    void ProcessEvents(const sf::Event& InEvent);

private:
    Scancode GetScancodeFromSFML(sf::Keyboard::Scan InScancode) const;
};

#endif // __INPUT_MANAGER_SFML_H__
