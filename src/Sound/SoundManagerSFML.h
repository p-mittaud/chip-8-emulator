#ifndef __SOUND_MANAGER_SFML_H__
#define __SOUND_MANAGER_SFML_H__

#include "SoundManager.h"
#include <memory>

namespace sf
{
    class SoundBuffer;
    class Sound;
}

class SoundManagerSFML : public SoundManager
{
public:
    SoundManagerSFML();
    SoundManagerSFML(const std::string& InFile);
    virtual ~SoundManagerSFML() = default;

    virtual bool LoadBeepSound(const std::string& InFile) override;
    virtual bool LoadSoundArray(unsigned char* bytesArray, size_t arraySize) override;
    virtual void SetPitch(float InPitch) override;

    virtual void PlayBeepSound(bool bShouldPlay) override;

private:
    std::shared_ptr<sf::SoundBuffer> BeepSoundBuffer;
    std::shared_ptr<sf::Sound> BeepSound;
};

#endif // __SOUND_MANAGER_SFML_H__
