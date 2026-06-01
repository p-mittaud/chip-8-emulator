#ifndef __INPUT_MANAGER_H___
#define __INPUT_MANAGER_H___

#include "Scancode.h"
#include <list>

class InputManager
{
public:
    InputManager() = default;
    virtual ~InputManager() = default;

    bool IsKeyPressed(unsigned char InKey) const { return KeysCurrentlyPressed[InKey]; }

    void ClearReleasedKeys() { KeysReleasedThisTick.clear(); }

    bool IsAnyKeyReleased() const { return !KeysReleasedThisTick.empty(); }
    unsigned char GetReleasedKey() const { return KeysReleasedThisTick.front(); }

protected:
    // Scancodes for input from 0x0 to 0xF
    int Scancodes[0x10]
    {
        (int)Scancode::X,       (int)Scancode::Num1,    (int)Scancode::Num2,    (int)Scancode::Num3,
        (int)Scancode::Q,       (int)Scancode::W,       (int)Scancode::E,       (int)Scancode::A,
        (int)Scancode::S,       (int)Scancode::D,       (int)Scancode::Z,       (int)Scancode::C,
        (int)Scancode::Num4,    (int)Scancode::R,       (int)Scancode::F,       (int)Scancode::V
    };

    std::list<unsigned char> KeysReleasedThisTick{};
    bool KeysCurrentlyPressed[0x10]{ false };
};

#endif // __INPUT_MANAGER_H___
