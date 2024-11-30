#pragma once
#include <AL/al.h>
#include <OgreVector3.h>
#include <OgreString.h>
#include <string>

class SoundBaseMgr;
namespace Ogre
{	class SceneNode;  }


//  SR3 dynamic sound for:
//  - ambient  (2d, stereo, track, looped)
//  - objects hit  (3d, mono, not looped)  destroyed after play

class SoundDynamic
{
	friend class SoundBaseMgr;

public:
	SoundDynamic(SoundBaseMgr* sound_mgr1);
	bool Create(std::string file, bool loop1=1, bool b2d=1, Ogre::SceneNode* nd=0);
	void Destroy();
	
	void Update(float dt);
	Ogre::SceneNode* node = 0;  // auto set pos and vel
	// Ogre::String name;

	void setPitch(float pitch);
	void setGain(float gain);

	void setLoop(bool loop);
	// void setEnabled(bool e);

	void play();
	void stop();

	bool getEnabled();
	bool isPlaying();
	
	void seek(float pos);  // [0..1)
	bool is2D =0;

//private:
	// void computeAudibility(Ogre::Vector3 pos);

	//  must not be changed during the lifetime of this object
	ALuint source = 0;  // not loaded
	ALuint buffer = 0;
	int samples =0;

	// float audibility = 0.f;
	float gain = 1.f;
	float pitch = 1.f;
	
	bool loop =0;
	bool enabled =0;  // not loaded
	// bool should_play =1;

	Ogre::Vector3 posOld{0,0,0};
	// Ogre::Vector3 vel{0,0,0};

	SoundBaseMgr* sound_mgr =0;
};
