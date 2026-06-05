#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <memory>

class InputManager;

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

    virtual void DrawDisplay(const bool* InDisplay, const int InWidth, const int InHeight) = 0;

    InputManager* GetInputManager() const { return InputManager.get(); }

protected:
    std::unique_ptr<InputManager> InputManager;
};

#endif // __WINDOW_H__
