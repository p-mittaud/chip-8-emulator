#include "SoundManagerSFML.h"

#include <SFML/Audio.hpp>
#include <SFML/Audio/Sound.hpp>

SoundManagerSFML::SoundManagerSFML()
{

}

bool SoundManagerSFML::LoadBeepSound(const std::string& InFile)
{
    BeepSoundBuffer = std::make_shared<sf::SoundBuffer>();
    if (!BeepSoundBuffer || !BeepSoundBuffer->loadFromFile(InFile))
    {
        return false;
    }

    BeepSound = std::make_shared<sf::Sound>(*BeepSoundBuffer);
    if (BeepSound)
    {
        BeepSound->setLooping(true);
    }

    return true;
}

void SoundManagerSFML::PlayBeepSound(bool bShouldPlay)
{
    if (BeepSound)
    {
        if (bShouldPlay)
        {
            BeepSound->play();
        }
        else
        {
            BeepSound->stop();
        }
    }
}
