#ifndef __WINDOW_SFML_H__
#define __WINDOW_SFML_H__

#include "Window.h"

#include <memory>
#include <cstdint>

namespace sf
{
    class RenderWindow;

    class Clock;
    class Time;
}

class InputManagerSFML;

class WindowSFML : public Window
{
public:
    WindowSFML(uint32_t InWidth, uint32_t InHeight, const WindowConfiguration& InConfig);
    virtual ~WindowSFML();

    virtual bool IsOpen() const override;
    virtual void ProcessEvents() override;

    virtual void UpdateTime() override;
    virtual bool ShouldRunFrame() const override;
    virtual void UpdateTimeSinceLastFrame() override;

    virtual void DrawDisplay(EmulatorDisplay InDisplay) override;

private:
    std::unique_ptr<sf::RenderWindow> Window;
    InputManagerSFML* InputMgrSFML{ nullptr };

    std::unique_ptr<sf::Clock> Clock;
    std::unique_ptr<sf::Time> TimePerFrame60Hz;
    std::unique_ptr<sf::Time> TimeSinceLastTimerUpdate;
};

#endif // __WINDOW_SFML_H__
