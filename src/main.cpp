#include <iostream>
#include <filesystem>
#include <ctime>

#include "Window/WindowSFML.h"
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

    auto Window = std::make_unique<WindowSFML>(width * pixelSize, height * pixelSize);

    std::cout << "Current path is " << std::filesystem::current_path() << '\n';

    std::string romFile("../resources/default.ch8");

    auto soundManager = std::make_unique<SoundManagerSFML>();
    soundManager->LoadBeepSound("../resources/sounds/beep.wav");

    // TODO: Add default constructor to directly load ROM
    Emulator emulator(Window->GetInputManager(), soundManager.get());
    emulator.LoadROM(romFile);

    // TODO: Add to config
    int instructionsPerSecond{700};
    int instructionsPerFrame = instructionsPerSecond / 60;

    // COSMAC VIP Quirk
    bool vBlankQuirkActive = true;

    while (Window->IsOpen())
    {
        Window->ProcessEvents();
        Window->UpdateTime();

        while (Window->ShouldRunFrame())
        {
            Window->UpdateTimeSinceLastFrame();

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

                Window->GetInputManager()->ClearReleasedKeys();
            }
        }
        
        Window->DrawDisplay(emulator.GetDisplay(), width, height);
    }

    return 0;
}