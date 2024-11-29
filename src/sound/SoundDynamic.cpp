#include "pch.h"
#include "Def_Str.h"
#include "SoundDynamic.h"
#include "SoundBaseMgr.h"
#include <AL/al.h>
using namespace Ogre;
using namespace std;


SoundDynamic::SoundDynamic(SoundBaseMgr* mgr)
	:sound_mgr(mgr)
{
}

bool SoundDynamic::isPlaying()
{
	if (enabled)
	{
		int value = 0;
		alGetSourcei(source, AL_SOURCE_STATE, &value);
		return value == AL_PLAYING;
	}
	return false;
}

bool SoundDynamic::getEnabled()
{
	return enabled;
}

void SoundDynamic::play()
{
	if (enabled)
		alSourcePlay(source);
}

void SoundDynamic::stop()
{
	if (enabled)
		alSourceStop(source);
}

void SoundDynamic::setGain(float gain1)
{
	if (!enabled)  return;
	if (gain == gain1)
		return;
	gain = gain1;
	alSourcef(source, AL_GAIN, gain * sound_mgr->master_volume);
}

void SoundDynamic::setLoop(bool loop1)
{
	if (!enabled)  return;
	if (loop == loop1)
		return;
	loop = loop1;
	alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
}

void SoundDynamic::setPitch(float pitch1)
{
	if (!enabled)  return;
	if (pitch == pitch1)
		return;
	pitch = pitch1;
	alSourcef(source, AL_PITCH, pitch);
}

void SoundDynamic::setPosition(Vector3 pos1)
{
	if (!enabled)  return;
	if (pos == pos1)
		return;
	pos = pos1;
	alSource3f(source, AL_POSITION, pos.x, pos.y, pos.z);
}

void SoundDynamic::setVelocity(Vector3 vel1)
{
	if (!enabled)  return;
	if (vel == vel1)
		return;
	vel = vel1;
	alSource3f(source, AL_VELOCITY, vel.x, vel.y, vel.z);
}

void SoundDynamic::seek(float pos)  // [0..1)
{
	if (!enabled)  return;
	alSourcei(source, AL_SAMPLE_OFFSET, pos * samples);
}

//  create
bool SoundDynamic::Create(string file, bool loop1, bool b2d, SceneNode* nd)
{
	if (enabled)
		return false;  // already

	alGenBuffers(1, &buffer);
	if (sound_mgr->hasALErrors())
		return false;


	//  load file  ----
	if (file.length() < 4)
	{
		LogO("@  Sound Create name too short: "+file);
		return false;
	}
	String ext = file.substr(file.length()-3);
	if (ext == "wav")
	{
		if (!sound_mgr->loadWAVFile(file, buffer, samples))
		{	//  error
			alDeleteBuffers(1, &buffer);
			enabled = false;
			return false;
		}
	}else if (ext == "ogg")
	{
		if (!sound_mgr->loadOGGFile(file, buffer, samples))
		{	//  error
			alDeleteBuffers(1, &buffer);
			enabled = false;
			return false;
		}
	}else
		LogO("@  Not supported sound file extension: "+ext);


	alGenSources(1, &source);
	if (sound_mgr->hasALErrors())
		return false;

	loop = loop1;
	is2D = b2d;
	node = nd;

	alSourcei(source, AL_BUFFER, buffer);
	if (sound_mgr->hasALErrors())
		return false;

	if (!is2D)
	{
		//  3d fade
		alSourcef(source, AL_REFERENCE_DISTANCE, sound_mgr->REF_DISTANCE);
		alSourcef(source, AL_ROLLOFF_FACTOR, sound_mgr->ROLLOFF_FACTOR);
		alSourcef(source, AL_MAX_DISTANCE, sound_mgr->MAX_DISTANCE);

		//  3d use reverb
		if (sound_mgr->reverb)
			alSource3i(source, AL_AUXILIARY_SEND_FILTER,
				sound_mgr->slot,  0, AL_FILTER_NULL);

		if (node)
		{	auto pos = node->getPosition();
			alSource3f(source, AL_POSITION, pos.x, pos.y, pos.z);
			alSource3f(source, AL_VELOCITY, 0.f, 0.f, 0.f);
		}
	}
	alSourcef(source, AL_GAIN, /*gain * */ sound_mgr->master_volume);
	alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
	alSourcef(source, AL_PITCH, 1.f);  //pitch);

	//  hud, 2d
	if (is2D)
	{
		alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
		alSource3f(source, AL_POSITION, 0.f,0.f,0.f);
		//alSourcef(source, AL_ROLLOFF_FACTOR, 0.f);
		alSourcePlay(source);
	}
	enabled = true;
	return true;
}

void SoundDynamic::Destroy()
{
	if (!enabled)
		return;
	alDeleteBuffers(1, &buffer);  buffer = 0;
	alDeleteSources(1, &source);  source = 0;
	enabled = false;
}

	
void SoundDynamic::Update()
{
	if (!node)  return;
	auto pos = node->getPosition();
	// todo: vel dt..
}
