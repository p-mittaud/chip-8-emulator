#include "SoundManagerSFML.h"

#include <SFML/Audio.hpp>
#include <SFML/Audio/Sound.hpp>

SoundManagerSFML::SoundManagerSFML()
{

}

SoundManagerSFML::SoundManagerSFML(const std::string& InFile)
{
    LoadBeepSound(InFile);
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

bool SoundManagerSFML::LoadSoundArray(unsigned char* bytesArray, size_t arraySize)
{
    constexpr int16_t SoundVolume = 10000;
    std::vector<int16_t> pcmSamples;
    pcmSamples.reserve(128); // 16 bytes * 8 bits

    for (auto i = 0; i < arraySize; ++i)
    {
        auto currentByte = bytesArray[i];

        for (int bit = 7; bit >= 0; --bit)
        {
            bool isBitSet = (currentByte >> bit) & 1;
            pcmSamples.push_back(isBitSet ? SoundVolume : -SoundVolume);
        }
    }

    if (!BeepSoundBuffer)
    {
        BeepSoundBuffer = std::make_shared<sf::SoundBuffer>();
    }

    auto channelMap = std::vector<sf::SoundChannel>{ sf::SoundChannel::Mono };
    if (!BeepSoundBuffer || !BeepSoundBuffer->loadFromSamples(pcmSamples.data(), pcmSamples.size(), channelMap.size(), 4000, channelMap))
    {
        return false;
    }

    if (!BeepSound)
    {
        BeepSound = std::make_shared<sf::Sound>(*BeepSoundBuffer);
        if (BeepSound)
        {
            BeepSound->setLooping(true);
        }
    }

    return true;
}

void SoundManagerSFML::SetPitch(float InPitch)
{
    if (BeepSound)
    {
        BeepSound->setPitch(InPitch);
    }
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
