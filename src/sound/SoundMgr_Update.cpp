#include "pch.h"
#include "Def_Str.h"
#include "SoundMgr.h"
#include "SoundBase.h"
#include "SoundBaseMgr.h"
#include <OgreDataStream.h>
#include <OgreException.h>
using namespace Ogre;


//  camera
void SoundMgr::setCamera(Vector3 position, Vector3 direction, Vector3 up, Vector3 velocity)
{
	if (disabled)  return;
	sound_mgr->setCamera(position, direction, up, velocity);
}

//  utility
//---------------------------------------------------------------------------------------
void SoundMgr::setPaused(bool mute)
{
	sound_mgr->pauseAll(mute);
}

void SoundMgr::setMasterVolume(float vol)
{
	sound_mgr->setMasterVolume(vol);
}


//  Pitch
//---------------------------------------------------------------------
void Sound::setPitch(float value)
{
	//if (start_sound)  // only pitch looped
	if (templ->free_sound == 0)  return;

	int ii = templ->free_sound;
	for (int i=0; i < ii; ++i)
		if (sounds[i])
		{
			float fq = templ->sound_pitches[i];
			float p = value / fq, v = 1.f;
			if (p > 2.f) {  p = 2.f;   v = 0.f;  }
			else
			if (p < 0.5f){  p = 0.5f;  v = 0.f;  }
			else
			if (p > 1.f) {  v = 1.f - (p - 1.f)*2.f;  if (v < 0.f)  v = 0.f;  }
			else
			if (p < 1.f) {  v = 1.f - (1.f - p)*2.f;  if (v < 0.f)  v = 0.f;  }
			
			if (engine && v < 0.001f)  v = 0.001f;  // engine always on
			if (ii == 1)  v = 1.f;
			
			pitch_gain[i] = v;
			sounds[i]->setGain(v * lastgain);
			sounds[i]->setPitch(p);
			//if (value > 100)  LogO(toStr(i)+" "+fToStr(p)+" "+fToStr(v));
		}
}

//  Gain
//---------------------------------------------------------------------
void Sound::setGain(float value)
{
	//if (fabs(lastgain - value) < 0.001f)
	//	return;
	float g = value * gain;
	
	if (start_sound)
		start_sound->setGain(g);

	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
			sounds[i]->setGain(g * pitch_gain[i]);

	if (stop_sound)
		stop_sound->setGain(g);

	lastgain = g;
}

void Sound::setEngine(bool b)
{
	engine = b;
}

void Sound::set2D(bool b)
{
	is2D = b;
	if (start_sound)
		start_sound->is2D = b;

	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
			sounds[i]->is2D = b;

	if (stop_sound)
		stop_sound->is2D = b;
}

void Sound::seek(float pos)
{
	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
			sounds[i]->seek(pos);
}

void Sound::setPosition(Vector3 pos, Vector3 velocity)
{
	if (start_sound)
	{	start_sound->setPosition(pos);
		start_sound->setVelocity(velocity);
	}
	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
		{	sounds[i]->setPosition(pos);
			sounds[i]->setVelocity(velocity);
		}
	if (stop_sound)
	{	stop_sound->setPosition(pos);
		stop_sound->setVelocity(velocity);
	}
}

bool Sound::isAudible()
{
	if (start_sound)
		return start_sound->isPlaying();

	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
			return sounds[i]->isPlaying();

	if (stop_sound)
		return stop_sound->isPlaying();

	return false;
}


//  Play
//---------------------------------------------------------------------
void Sound::runOnce()
{
	if (start_sound)
	if (start_sound->isPlaying())  return;
		else start_sound->play();

	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
		{
			if (sounds[i]->isPlaying())
				continue;

			sounds[i]->setLoop(false);
			sounds[i]->play();
		}

	if (stop_sound)
	if (stop_sound->isPlaying())  return;
		else stop_sound->play();
}

void Sound::start()
{
	if (start_sound)
	{	start_sound->stop();
		start_sound->play();
	}
	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
		{	sounds[i]->setLoop(true);
			sounds[i]->play();
		}
}

void Sound::stop()
{
	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
			sounds[i]->stop();

	if (stop_sound)
	{	stop_sound->stop();
		stop_sound->play();
	}
}

void Sound::kill()
{
	for (int i = 0; i < templ->free_sound; ++i)
		if (sounds[i])
			sounds[i]->stop();

	if (start_sound)
		start_sound->stop();

	if (stop_sound)
	{	stop_sound->stop();
		stop_sound->play();
	}
}

void Sound::setEnabled(bool e)
{
	if (start_sound)
		start_sound->setEnabled(e);

	if (stop_sound)
		stop_sound->setEnabled(e);

	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
			sounds[i]->setEnabled(e);
}


//  update
void SoundMgr::Update(float dt)
{
	if (sound_mgr && !disabled)
		sound_mgr->Update(dt);
}
