#include <iostream>
#include <ctime>

#include "Window/WindowSFML.h"
#include "Input/InputManagerSFML.h"
#include "Sound/SoundManagerSFML.h"

#include "Emulator.h"

#include <toml++/toml.hpp>

int main()
{
    srand( (unsigned)time(NULL) );

    std::string FileROM{}, BeepSound{};
    uint32_t InstructionsPerSecond{ 700 }, PixelSize{ 10 };
    WindowConfiguration WindowConfig{};
    int EmulatorType{ 1 };

    try {
        auto config = toml::parse_file("../config.toml");

        // Emulator Config
        EmulatorType = config["Emulator"]["Type"].value_or(EmulatorType);
        FileROM = config["Emulator"]["FileROM"].value_or("");
        BeepSound = config["Emulator"]["BeepSound"].value_or("");
        InstructionsPerSecond = config["Emulator"]["InstructionsPerSecond"].value_or(InstructionsPerSecond);

        // Window Config
        WindowConfig.PixelSize = config["Window"]["PixelSize"].value_or(PixelSize);
        WindowConfig.OffColor.r = config["Window"]["OffColor"][0].value_or(0);
        WindowConfig.OffColor.g = config["Window"]["OffColor"][1].value_or(0);
        WindowConfig.OffColor.b = config["Window"]["OffColor"][2].value_or(0);
        WindowConfig.OnColor.r = config["Window"]["OnColor"][0].value_or(0);
        WindowConfig.OnColor.g = config["Window"]["OnColor"][1].value_or(0);
        WindowConfig.OnColor.b = config["Window"]["OnColor"][2].value_or(0);
    }
    catch (const toml::parse_error& err) {
        std::cerr << "Failed to load TOML config file: " << err.description() << "\n";
        return 1;
    }

    // Get dimensions from emulator type
    constexpr uint32_t width = 64u;
    constexpr uint32_t height = 32u;

    auto Window = std::make_unique<WindowSFML>(width, height, WindowConfig);

    auto soundManager = std::make_unique<SoundManagerSFML>(BeepSound);

    Emulator emulator(Window->GetInputManager(), soundManager.get(), FileROM, EmulatorType);

    int instructionsPerFrame = InstructionsPerSecond / 60;

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
        
        Window->DrawDisplay(emulator.GetDisplay());
    }

    return 0;
}