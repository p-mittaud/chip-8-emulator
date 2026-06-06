#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <memory>
#include <cstdint>

class InputManager;

struct EmulatorDisplay;

struct Color
{
    uint8_t r, g, b;
};

struct WindowConfiguration
{
    Color OffColor{ 0, 0, 0 };
    Color OnColor{ 255, 255, 255 };

    uint32_t PixelSize{ 10u };
};

class Window
{
public:
    Window() = default;
    virtual ~Window() = default;

    virtual bool IsOpen() const = 0;
    virtual void ProcessEvents() = 0;

    virtual void UpdateTime() = 0;
    virtual bool ShouldRunFrame() const = 0;
    virtual void UpdateTimeSinceLastFrame() = 0;

    virtual void DrawDisplay(EmulatorDisplay InDisplay) = 0;

    InputManager* GetInputManager() const { return InputManager.get(); }

protected:
    std::unique_ptr<InputManager> InputManager;

    WindowConfiguration WindowConfig;
};

#endif // __WINDOW_H__
