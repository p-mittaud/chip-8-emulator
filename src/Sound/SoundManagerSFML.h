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
    virtual ~SoundManagerSFML() = default;

    virtual bool LoadBeepSound(const std::string& InFile);

    virtual void PlayBeepSound(bool bShouldPlay);

private:
    std::shared_ptr<sf::SoundBuffer> BeepSoundBuffer;
    std::shared_ptr<sf::Sound> BeepSound;
};

#endif // __SOUND_MANAGER_SFML_H__
