#ifndef __SOUND_MANAGER_H__
#define __SOUND_MANAGER_H__

#include <string>

class SoundManager
{
public:
    SoundManager() = default;
    virtual ~SoundManager() = default;

    virtual bool LoadBeepSound(const std::string& InFile) = 0;

    virtual void PlayBeepSound(bool bShouldPlay) = 0;
};

#endif // __SOUND_MANAGER_H__
