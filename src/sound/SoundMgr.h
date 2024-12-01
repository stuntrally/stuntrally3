//  based on RoR's  /source/main/audio/SoundScriptManager.h
//  https://github.com/RigsOfRods/rigs-of-rods
#pragma once
#include <OgreString.h>
#include <OgreStringVector.h>
#include <map>

namespace Ogre  {  class FileStreamDataStream;  class SceneNode;  }
class SoundBase;  class SoundBaseMgr;  class SETTINGS;

const int MAX_SOUNDS_PER_SCRIPT = 12;  // per 1 template, todo? vector<


///  sound template  from .cfg to create
//---------------------------------------------------------------------------------------
class SoundTemplate
{
	friend class SoundMgr;
	friend class Sound;

public:
	SoundTemplate(Ogre::String name, Ogre::String filename);
	
private:
	bool setParameter(Ogre::StringVector vec);

	Ogre::String name, file_name;

	bool  has_start =0, has_stop =0;
	bool  unpitchable =0;

	Ogre::String sound_names[MAX_SOUNDS_PER_SCRIPT];
	float        sound_pitches[MAX_SOUNDS_PER_SCRIPT];
	Ogre::String start_name, stop_name;

	int   free_sound = 0;  // for names[, pitches[
};


///  Sound instance
//---------------------------------------------------------------------------------------
class Sound
{
	friend class SoundMgr;
public:
	static int instances;

	Sound(SoundTemplate* tpl, SoundBaseMgr* mgr);
	~Sound();

	void setGain(float value);
	void setPitch(float value);
	void setPosition(Ogre::Vector3 pos, Ogre::Vector3 velocity);

	bool isAudible();
	void start(), stop(), kill();
	bool is2D;  // hud sounds, no distance attenuation
	void set2D(bool b), setEngine(bool b);

	void seek(float pos);
	void runOnce();
	void setEnabled(bool e);

private:
	SoundTemplate* templ =0;
	SoundBaseMgr* sound_mgr =0;

	SoundBase* start_sound =0, *stop_sound =0;
	SoundBase* sounds[MAX_SOUNDS_PER_SCRIPT];

	float pitch_gain[MAX_SOUNDS_PER_SCRIPT];
	float lastgain;

	bool engine;
};


///  Sounds manager
//---------------------------------------------------------------------------------------
class SoundMgr
{
public:
	SoundMgr(SETTINGS* pSet1);
	~SoundMgr();
	SETTINGS* pSet =0;

	bool Init(std::string snd_device, bool reverb);
	void Update(float dt);

	void parseScript(Ogre::FileStreamDataStream* stream);  // sounds.cfg


	//  template name from .cfg
	Sound* createInstance(Ogre::String templ);  // new Sound

	void CreateAmbient(Ogre::String templ);
	void DestroyAmbient();

	void CreateDynamic(Ogre::String templ, Ogre::SceneNode* node);


	void setPaused(bool mute);
	void setMasterVolume(float vol);

	void setCamera(Ogre::Vector3 position, Ogre::Vector3 direction, Ogre::Vector3 up, Ogre::Vector3 velocity);
	bool isDisabled() {  return disabled;  }

	SoundBaseMgr* sound_mgr =0;

private:
	SoundTemplate* createTemplate(Ogre::String name, Ogre::String filename);
	void skipToNextCloseBrace(Ogre::FileStreamDataStream* chunk);
	void skipToNextOpenBrace(Ogre::FileStreamDataStream* chunk);

	bool disabled =1;

	std::map <Ogre::String, SoundTemplate*> templates;
	std::vector<SoundTemplate*> v_templ;  // to delete
};
