/*
    This source file is part of Rigs of Rods
    Copyright 2005-2012 Pierre-Michel Ricordel
    Copyright 2007-2012 Thomas Fischer
    Copyright 2017-2018 Petr Ohlidal

    For more information, see http://www.rigsofrods.org/

    Rigs of Rods is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 3, as
    published by the Free Software Foundation.

    Rigs of Rods is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rigs of Rods. If not, see <http://www.gnu.org/licenses/>.
*/
//  Modified by CryHam for SR3


#pragma once

// #include "Application.h"
#include "RefCountingObjectPtr.h"
#include "Sound.h"
#include "SoundManager.h"
// #include <OgreScriptLoader.h>

#include <AL/efx-presets.h>


enum {
    MAX_SOUNDS_PER_SCRIPT = 16,
    MAX_INSTANCES_PER_GROUP = 256
};

enum SoundTriggers {
    SS_TRIG_NONE = -1,
    SS_TRIG_ENGINE = 0,
    SS_TRIG_AEROENGINE1,
    SS_TRIG_AEROENGINE2,
    SS_TRIG_BRAKE,
    SS_TRIG_STARTER,
    SS_TRIG_TURBOBOV,
    SS_TRIG_TURBOWASTEGATE,
    SS_TRIG_TURBOBACKFIRE,
    SS_TRIG_ALWAYSON,
    SS_TRIG_REPAIR,
    SS_TRIG_AIR,
    SS_TRIG_SHIFT,
    SS_TRIG_GEARSLIDE,
    SS_TRIG_CREAK,
    SS_TRIG_SCREETCH,
    SS_TRIG_AFTERBURNER1,
    SS_TRIG_AFTERBURNER2,
    SS_TRIG_IGNITION,
    SS_TRIG_REVERSE_GEAR,
    SS_TRIG_LINKED_COMMAND,
    SS_TRIG_MAIN_MENU,
    SS_MAX_TRIG
};

enum ModulationSources {
    SS_MOD_NONE,
    SS_MOD_ENGINE,
    SS_MOD_TURBO,
    SS_MOD_AEROENGINE1,
    SS_MOD_AEROENGINE2,
    SS_MOD_WHEELSPEED,
    SS_MOD_GEARBOX,
    SS_MOD_CREAK,
    SS_MOD_BREAK,
    SS_MOD_SCREETCH,
    SS_MOD_THROTTLE1,
    SS_MOD_THROTTLE2,
    SS_MOD_AIRSPEED,
    SS_MOD_LINKED_COMMANDRATE,
    SS_MOD_MUSIC_VOLUME,
    SS_MAX_MOD
};

enum SoundLinkTypes {
    SL_DEFAULT,
    SL_COMMAND, 
    SL_HYDRO, 
    SL_COLLISION, 
    SL_SHOCKS, 
    SL_BRAKES, 
    SL_ROPES, 
    SL_TIES, 
    SL_PARTICLES, 
    SL_AXLES, 
    SL_FLARES, 
    SL_FLEXBODIES, 
    SL_EXHAUSTS, 
    SL_VIDEOCAMERA, 
    SL_MAX
};


//  Template
//-----------------------------------------------------------------------------------------------------
class SoundScriptTemplate : public RefCountingObject<SoundScriptTemplate>
{
    friend class SoundScriptManager;
    friend class SoundScriptInstance;

public:
    SoundScriptTemplate(Ogre::String name, Ogre::String groupname, Ogre::String filename, bool baseTemplate);
    virtual ~SoundScriptTemplate() override {};

    // 'sound' attribute getters for AngelScript
    int getNumSounds() {  return free_sound;  }
    Ogre::String getSoundName(int pos)
    {
        if (pos >= 0 && pos < free_sound)
            return sound_names[pos];
        else
            return "";
    }
    float getSoundPitch(int pos)
    {
        if (pos >= 0 && pos < free_sound)
            return sound_pitches[pos];
        else
            return 0.f;
    }

    // start/stop sound attribute getters for AngelScript
    Ogre::String getStartSoundName() {  return start_sound_name;  }
    float getStartSoundPitch()       {  return start_sound_pitch;  }
    Ogre::String getStopSoundName()  {  return stop_sound_name;  }
    float getStopSoundPitch()        {  return stop_sound_pitch;  }

    // other getters for AngelScript
    Ogre::String getName()      {  return name;  }
    Ogre::String getFileName()  {  return file_name;  }
    Ogre::String getGroupName() {  return group_name;  }
    bool isBaseTemplate()       {  return base_template;  }

private:
    int parseModulation(Ogre::String str);
    bool setParameter(Ogre::StringVector vec);

    Ogre::String name;
    Ogre::String file_name;
    Ogre::String group_name;

    bool         base_template;
    bool         has_start_sound;
    bool         has_stop_sound;
    bool         unpitchable;

    float        gain_multiplier;
    float        gain_offset;
    float        gain_square;
    int          gain_source;

    float        pitch_multiplier;
    float        pitch_offset;
    float        pitch_square;
    int          pitch_source;

    Ogre::String sound_names[MAX_SOUNDS_PER_SCRIPT];
    float        sound_pitches[MAX_SOUNDS_PER_SCRIPT];
    Ogre::String start_sound_name;
    float        start_sound_pitch;
    Ogre::String stop_sound_name;
    float        stop_sound_pitch;

    int          trigger_source;
    int          free_sound;
};


//  Instance
//-----------------------------------------------------------------------------------------------------
class SoundScriptInstance : public RefCountingObject<SoundScriptInstance>
{
    friend class SoundScriptManager;

public:
    SoundScriptInstance(int actor_id, SoundScriptTemplatePtr templ,
        SoundManager* sm, Ogre::String instancename,
        int soundLinkType = SL_DEFAULT, int soundLinkItemId = -1);
    virtual ~SoundScriptInstance() override
    {   };

    void runOnce();
    void setEnabled(bool e);
    void setGain(float value);
    void setPitch(float value);
    void setPosition(Ogre::Vector3 pos);
    void setVelocity(Ogre::Vector3 velo);
    void start();
    void stop();
    void kill();

    SoundScriptTemplatePtr getTemplate() {  return templ;  }
    const SoundPtr& getStartSound()      {  return start_sound;  }
    const SoundPtr& getStopSound()       {  return stop_sound;  }
    const SoundPtr& getSound(int pos)
    {  
        if (pos >= 0 && pos < templ->free_sound)
            return sounds[pos];
        else
            return SOUNDPTR_NULL;
    }
    float getStartSoundPitchgain() {  return start_sound_pitchgain;  }
    float getStopSoundPitchgain()  {  return stop_sound_pitchgain;  }
    float getSoundPitchgain(int pos)
    {
        if (pos >= 0 && pos < templ->free_sound)
            return sounds_pitchgain[pos];
        else
            return 0.f;
    }
    int getActorInstanceId()              {  return actor_id;  }
    const Ogre::String& getInstanceName() {  return instance_name;  }

    static const float PITCHDOWN_FADE_FACTOR;
    static const float PITCHDOWN_CUTOFF_FACTOR;
    static const int ACTOR_ID_UNKNOWN = -1;
    static const int ACTOR_ID_TERRAIN_OBJECT = -2;
    static const SoundPtr SOUNDPTR_NULL; // Dummy value to be returned as const reference.

private:
    float pitchgain_cutoff(float sourcepitch, float targetpitch);

    Ogre::String instance_name;
    SoundScriptTemplatePtr templ;
    SoundManager* sound_manager;
    SoundPtr start_sound;
    SoundPtr stop_sound;
    SoundPtr sounds[MAX_SOUNDS_PER_SCRIPT];
    float start_sound_pitchgain;
    float stop_sound_pitchgain;
    float sounds_pitchgain[MAX_SOUNDS_PER_SCRIPT];
    float lastgain;

    int actor_id;           // ID of the actor this sound belongs to, or an `ACTOR_ID_*` constant.
    int sound_link_type;    // holds the SL_ type this is bound to
    int sound_link_item_id; // holds the item number this is for
};


//  Manager
//-----------------------------------------------------------------------------------------------------
class SoundScriptManager : public Ogre::ScriptLoader
{
public:
    SoundScriptManager();
    ~SoundScriptManager();

    // ScriptLoader interface
    const Ogre::StringVector& getScriptPatterns() const;
    void parseScript(Ogre::DataStreamPtr& stream, const Ogre::String& groupName);
    Ogre::Real getLoadingOrder() const;

    SoundScriptInstancePtr createInstance(Ogre::String templatename,
        int actor_id = 0, int soundLinkType = SL_DEFAULT, int soundLinkItemId = -1);
    void removeInstance(const SoundScriptInstancePtr& ssi);

    std::vector<SoundScriptInstancePtr>& getAllInstances() {  return instances;  }
    SoundScriptTemplatePtr getTemplate(Ogre::String name)  {  return templates[name];  }
    std::map <Ogre::String, SoundScriptTemplatePtr>& getAllTemplates() {  return templates;  }

    // functions
    void trigOnce    (int actor_id, int trig, int linkType = SL_DEFAULT, int linkItemID=-1);
    void trigStart   (int actor_id, int trig, int linkType = SL_DEFAULT, int linkItemID=-1);
    void trigStop    (int actor_id, int trig, int linkType = SL_DEFAULT, int linkItemID=-1);
    void trigToggle  (int actor_id, int trig, int linkType = SL_DEFAULT, int linkItemID=-1);
    void trigKill	 (int actor_id, int trig, int linkType = SL_DEFAULT, int linkItemID=-1);
    bool getTrigState(int actor_id, int trig, int linkType = SL_DEFAULT, int linkItemID=-1);
    void modulate    (int actor_id, int mod, float value, int linkType = SL_DEFAULT, int linkItemID=-1);

    void setEnabled(bool state);

    void SetDopplerFactor(float doppler_factor);
    void SetListener(Ogre::Vector3 position, Ogre::Vector3 direction, Ogre::Vector3 up, Ogre::Vector3 velocity);
    void setLoadingBaseSounds(bool value) {  loading_base = value;  };

    bool isDisabled() {  return disabled;  }

    // void update(float dt_sec, Ogre::SceneNode* camera_node);
    void update(float dt_sec, Ogre::Camera* camera_node);

    //  True if the listener position is below water level. False otherwise.
    bool ListenerUnderwater() const {  return listener_underwater;  }

    //  True if the listener position is inside the AABB of the actor the player character is coupled to. False otherwise.
    bool ListenerInsidePlayer() const {  return listener_inside_player;  }

    SoundManager* getSoundManager() {  return sound_manager;  }

private:
    SoundScriptTemplatePtr createTemplate(Ogre::String name, Ogre::String groupname, Ogre::String filename);
    void skipToNextCloseBrace(Ogre::DataStreamPtr& chunk);
    void skipToNextOpenBrace(Ogre::DataStreamPtr& chunk);

    bool disabled;
    bool loading_base;
    bool listener_underwater = false;
    bool listener_inside_player = false;
    float max_distance;
    float reference_distance;
    float rolloff_factor;
    int instance_counter;
    Ogre::StringVector script_patterns;

    std::map <Ogre::String, SoundScriptTemplatePtr> templates;
    std::vector<SoundScriptInstancePtr> instances;

    //  instances lookup tables - using `std::array<>` because it does bounds checking under VisualStudio/Debug.
    std::array<int, SS_MAX_TRIG> free_trigs;
    std::array<SoundScriptInstancePtr, SS_MAX_TRIG* MAX_INSTANCES_PER_GROUP> trigs;

    std::array<int, SS_MAX_MOD> free_pitches;
    std::array<SoundScriptInstancePtr, SS_MAX_MOD* MAX_INSTANCES_PER_GROUP> pitches;
    
    std::array<int, SS_MAX_MOD> free_gains;
    std::array<SoundScriptInstancePtr, SS_MAX_MOD* MAX_INSTANCES_PER_GROUP> gains;

    //  state map
    //  soundLinks, soundItems, actor_ids, triggers
    std::map <int, std::map <int, std::map <int, std::map <int, bool > > > > state_map;

    /**
     * Determines which environment in terms of reverb corresponds to the provided position and returns
     * its properties.
     * @return Reverb properties for the provided position.
     */
    const EFXEAXREVERBPROPERTIES* GetReverbPresetAt(Ogre::Vector3 position) const;
    void SetListenerEnvironment(Ogre::Vector3 position);

    SoundManager* sound_manager;
};
