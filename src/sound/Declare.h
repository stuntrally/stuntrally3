#pragma once
#include "Def_Str.h"
#include "RefCountingObjectPtr.h"

#include <limits>
#include <vector>

#define LOG(s)  LogO(s)
#define TOSTRING(i)  Ogre::StringConverter::toString(i)


// for std::vector
template <class T, class A, class Predicate>
inline void EraseIf(std::vector<T, A>& c, Predicate pred)
{
    c.erase(std::remove_if(c.begin(), c.end(), pred), c.end());
}

class Audio  // RoR's  App  audio config
{
public:
    static float audio_master_volume;
    static bool audio_enable_creak;
    static bool audio_enable_obstruction;
    static bool audio_enable_reflection_panning;
    static std::string audio_device_name;
    static float audio_doppler_factor;
    static bool audio_menu_music;
    static bool audio_enable_efx;
    static bool audio_engine_controls_environmental_audio;
    static int audio_efx_reverb_engine;
    static std::string audio_default_listener_efx_preset;
    static std::string audio_force_listener_efx_preset;
};


enum /*class*/ EfxReverbEngine
{
    NONE,
    REVERB,
    EAXREVERB,
};


class Actor;
class Sound;
class SoundManager;
class SoundScriptInstance;
class SoundScriptManager;
class SoundScriptTemplate;

typedef RefCountingObjectPtr<Actor> ActorPtr;
typedef RefCountingObjectPtr<Sound> SoundPtr;
typedef RefCountingObjectPtr<SoundScriptInstance> SoundScriptInstancePtr;
typedef RefCountingObjectPtr<SoundScriptTemplate> SoundScriptTemplatePtr;
