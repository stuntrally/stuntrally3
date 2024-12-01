//  based on RoR's  /source/main/audio/SoundManager.h
//  https://github.com/RigsOfRods/rigs-of-rods
#pragma once
#include <Ogre.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
class SoundBase;  class SoundDynamic;  struct REVERB_PRESET;  class SETTINGS;

//#define REVERB_BROWSER  // 🧰 _Tool_  keys 1,2: change prev,next reverb preset


class SoundBaseMgr
{
	friend class SoundBase;
public:
	SoundBaseMgr(SETTINGS* pSet1);
	~SoundBaseMgr();
	SETTINGS* pSet =0;

	//  main  ---
	bool Init(std::string snd_device, bool reverb1);
	void CreateSources(), DestroySources(bool all=false);  // for game reload
	void Update(float dt);

	SoundBase* createSound(Ogre::String file, Ogre::String name);

	void setCamera(Ogre::Vector3 position, Ogre::Vector3 direction, Ogre::Vector3 up, Ogre::Vector3 velocity);
	void pauseAll(bool mute);
	void setMasterVolume(float vol);

	bool isDisabled() {  return device == 0;  }


	//  reverb  ---
	void SetReverb(std::string name);

	bool reverb =1;
	std::string sReverb;  // info
	void InitReverbMap();
	std::map <std::string, int> mapReverbs;
	ALuint LoadEffect(const REVERB_PRESET* reverb);

	//  var
	int hw_sources_num = 0;  // total number of available hardware sources < ALL_SRC
	int hw_sources_use = 0;
	int sources_use = 0;
	int buffers_use = 0, buffers_used_max = 0;

//private:
	void recomputeAllSources();
	void recomputeSource(int source_id, int reason, float vfl, Ogre::Vector3* vvec);
	ALuint getHwSource(int hw_id) {  return hw_sources[hw_id];  };

	void assign(int source_id, int hw_id);
	void retire(int source_id);

	//  load file, return false on error
	bool loadWAVFile(Ogre::String file, ALuint buffer, int& outSamples);
	bool loadOGGFile(Ogre::String file, ALuint buffer, int& outSamples);

	
	//  SR3 new, dynamic sounds  ----
	SoundDynamic* ambient =0;  // one, 2d, looped, always play

	std::list<SoundDynamic*> dynamics;  // auto removed
	int cnt_dynamic = 0;  // stat, playing


	//  audio sources  ----
	std::vector<SoundBase*> sources;

	//  active audio hardware sources
	std::vector<int>  hw_sources_map;   // stores the hardware index for each source. -1 = unmapped
	std::vector<ALuint> hw_sources;     // this buffer contains valid AL handles up to hw_sources_num


	//  helper for calculating the most audible sources
	std::vector<std::pair<int, float>> src_audible;
	
	//  audio buffers: Array of AL buffers and filenames
	std::vector<ALuint>  buffers;
	std::vector<Ogre::String> buffer_file;

	Ogre::Vector3 camera_position{0,0,0};

	//  AL vars  ----
	ALCdevice*  device =0;
	ALCcontext* context =0;

	ALuint slot =0, effect =0;

	float master_volume = 1.f;


	//  AL function pointers
	bool _checkALErrors(const char* file, int line);
private:
	LPALGENEFFECTS alGenEffects;
	LPALDELETEEFFECTS alDeleteEffects;
	LPALISEFFECT alIsEffect;
	LPALEFFECTI alEffecti;
	LPALEFFECTF alEffectf;
	LPALEFFECTFV alEffectfv;
	LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
	LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
	LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
};

#define hasALErrors()  _checkALErrors(__FILE__, __LINE__)
