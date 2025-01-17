#include "pch.h"
#include "Def_Str.h"
#include "SoundBase.h"
#include "SoundBaseMgr.h"
#include "SoundDynamic.h"
#include "SoundReverbSets.h"
#include "SceneClasses.h"
#include "settings.h"

#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/efx.h>
#include <vector>
using namespace Ogre;
using namespace std;


//  Init
//---------------------------------------------------------------------------------------------
SoundBaseMgr::SoundBaseMgr(SETTINGS* pSet1)
	:pSet(pSet1)
{
	hw_sources_map.resize(pSet->s.cnt_sources,0);
	hw_sources.resize(pSet->s.cnt_sources,0);

	src_audible.resize(pSet->s.cnt_buffers);
	sources.resize(pSet->s.cnt_buffers,0);
	buffers.resize(pSet->s.cnt_buffers,0);
	buffer_file.resize(pSet->s.cnt_buffers);

	// MAX_DISTANCE = 500.f;  // 500
	// REF_DISTANCE = 1.0f;   // 1  7.5 new
	// ROLLOFF_FACTOR = 0.05f;  // 0.05 0.1  1.0 new
}

void logList(const char *list)
{
	if (!list || *list == '\0')
		LogO("@@@  None!");
	else
	do
	{	LogO(String("@  ") + list);
		list += strlen(list) + 1;
	}
	while (*list != '\0');
}

bool SoundBaseMgr::Init(std::string snd_device, bool reverb1)
{
	reverb = reverb1;

	//  list devices
	LogO("@@  ---- Sound devices ----");
	if (alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT") != AL_FALSE)
		logList(alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER));
	else
		logList(alcGetString(NULL, ALC_DEVICE_SPECIFIER));

	
	//  open device
	if (snd_device == "")
		device = alcOpenDevice(NULL);
	else
		device = alcOpenDevice(snd_device.c_str());

	if (!device)
	{
		LogO("@@@  Sound Init - Could not open device");
		hasALErrors();
		return false;
	}

	//  efx
	ALCboolean efx = alcIsExtensionPresent(device, ALC_EXT_EFX_NAME);
	if (efx == ALC_FALSE)		LogO("@  EFX extention not found !");
	else if (efx == ALC_TRUE)	LogO("@  EFX extension found.");

	std::vector<ALint> attr;
	attr.push_back(ALC_MONO_SOURCES);  attr.push_back(pSet->s.cntAll());
	attr.push_back(ALC_STEREO_SOURCES);  attr.push_back(pSet->s.cnt_sources / 10);  //?..
	if (reverb)
	{	attr.push_back(ALC_MAX_AUXILIARY_SENDS);  attr.push_back(4);  }
	attr.push_back(0);  // end array

	//  context
	context = alcCreateContext(device, &attr[0]);
	if (context == NULL ||
		alcMakeContextCurrent(context) == ALC_FALSE)
	{
		LogO("@@@  Sound Init - Could not create context");
		if (context != NULL)
			alcDestroyContext(context);
		alcCloseDevice(device);
		device = NULL;
		hasALErrors();
		return false;
	}

	
	//  log info  ----
	String s,t;
	LogO("@@  ---- SoundManager Info ----");
	s = alGetString(AL_VENDOR);		LogO("@  vendor: " + s);
	s = alGetString(AL_VERSION);	LogO("@  version: " + s);
	s = alGetString(AL_RENDERER);	LogO("@  renderer: " + s);
	//t = alcGetString(device, ALC_DEVICE_SPECIFIER);	LogO("@  renderer: " + s + "  alc device: " + t);
	s = alGetString(AL_EXTENSIONS);	LogO("@  extensions: " + s);
	//t = alcGetString(device, ALC_EXTENSIONS);	LogO("@  alc extensions: " + s);


	//  sends
	ALint iSends = 0;
	alcGetIntegerv(device, ALC_MAX_AUXILIARY_SENDS, 1, &iSends);
	LogO("@  Aux Sends per Source: " + toStr(iSends));

   
	//  get function pointers
	alGenEffects    = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
	alDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
	
	alIsEffect = (LPALISEFFECT)alGetProcAddress("alIsEffect");
	alEffecti  = (LPALEFFECTI)alGetProcAddress("alEffecti");
	alEffectf  = (LPALEFFECTF)alGetProcAddress("alEffectf");
	alEffectfv = (LPALEFFECTFV)alGetProcAddress("alEffectfv");
	
	alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
	alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
	alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");


	//  doppler
	alDopplerFactor(0.f);  // 1.f  todo: vel..
	alSpeedOfSound(343.3f);
	/*	// alDopplerVelocity(1.f);
		// SetAirAbsorptionFactor(1.0f);
        if (IsUnderwater())
            SetSpeedOfSound(1522.0f); // assume listener is in sea water (i.e. salt water)
            SetAirAbsorptionFactor(0.00668f);
            alSourcef(source, AL_AIR_ABSORPTION_FACTOR, m_air_absorption_factor);
	*/
	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);  //+

	//  reverb
	if (!reverb)
	{
		LogO("@  Not using reverb.");
		return true;
	}

	//  slot is what plays an effect on sources that connect to it
	alGenAuxiliaryEffectSlots(1, &slot);
	InitReverbMap();
	
	//SetReverb("MOUNTAINS");
	return true;
}


//  🔊 Reverb
//-----------------------------------------------------------------------------------
void SoundBaseMgr::SetReverb(std::string name)
{
	if (!device || !reverb)  return;
	sReverb = name;
	int r = mapReverbs[name] -1;
	if (r < 0 || r >= RVB_PRESETS_ALL)
	{
		r = RVB_GENERIC;  // use generic
		LogO("@  Reverb preset not found! "+name);
		sReverb = "GENERIC, not found";
	}
	const REVERB_PRESET* reverb = &ReverbPresets[r];

	effect = LoadEffect(reverb);
	if (!effect)
		LogO("@  Can't load effect !!");

	//  Tell the effect slot to use the loaded effect object.
	//  Note that the this copies the effect properties.
	alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, effect);
	if (alGetError() != AL_NO_ERROR)
		LogO("@  Failed to set effect slot!");
	alDeleteEffects(1, &effect);
}


//  🆕 Create  --
void SoundBaseMgr::CreateSources()
{
	if (!device)  return;
	LogO("@ @  Creating hw sources.");
	
	int i, ss = pSet->s.cnt_sources;
	for (i = 0; i < ss; ++i)
	{
		alGetError();
		alGenSources(1, &hw_sources[i]);
		//alSource3i(source, AL_AUXILIARY_SEND_FILTER, slot, 0, AL_FILTER_NULL);

		if (alGetError() != AL_NO_ERROR)  break;
		alSourcef(hw_sources[i], AL_REFERENCE_DISTANCE, pSet->s.ref_dist);
		alSourcef(hw_sources[i], AL_ROLLOFF_FACTOR, pSet->s.rolloff);
		alSourcef(hw_sources[i], AL_MAX_DISTANCE, pSet->s.max_dist);
		//LogO(toStr(i)+" +SRC: "+toStr(hw_sources[i]));
		++hw_sources_num;
	}
	LogO("@ @  Created hw sources: "+toStr(hw_sources_num)+" / "+toStr(ss));

	for (i = 0; i < ss; ++i)
		hw_sources_map[i] = -1;

	buffers_used_max = buffers_use;  // save for info
	buffers_use = 0;  //)+ zero after all loaded
}

//  💥 Destroy  --
void SoundBaseMgr::DestroySources(bool all)
{
	if (!device)  return;
	
	LogO("@ @  Destroying hw sources.");
	int i;
	for (int i = 0; i < pSet->s.cnt_sources; ++i)
	{
		//LogO(toStr(i)+" -SRC: "+toStr(hw_sources[i]));
		alSourceStop(hw_sources[i]);
		alSourcei(hw_sources[i], AL_BUFFER, 0);
		alDeleteSources(1, &hw_sources[i]);
		--hw_sources_num;
	}

	//buffers_use = 0;  //)+ needed when loading, zero after CreateSources
	hw_sources_use = 0;
}

//  💥 Destroy
SoundBaseMgr::~SoundBaseMgr()
{
	if (device)
	{
		if (reverb)
			alDeleteAuxiliaryEffectSlots(1, &slot);

		//  sources and buffers
		DestroySources(true);
		alDeleteBuffers(pSet->s.cnt_buffers, &buffers[0]);
	}

	//  context and device
	ALCcontext* context = alcGetCurrentContext();
	if (context == NULL)
	{	LogO("@ @  SoundManager was disabled.");
		return;
	}
	ALCdevice* device = alcGetContextsDevice(context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	if (device)
		alcCloseDevice(device);

	LogO("@ @  SoundManager destroyed.");
}


bool compareByAudibility(std::pair<int, float> a, std::pair<int, float> b)
{
	return a.second > b.second;
}

//  💫 called when camera moves
void SoundBaseMgr::recomputeAllSources()
{
	if (!device)  return;

	int i;
	for (i=0; i < sources_use; i++)
	{
		sources[i]->computeAudibility(camera_position);
		src_audible[i].first = i;
		src_audible[i].second = sources[i]->audibility;
	}

	//  sort first 'num_hardware_sources' sources by audibility
	//  see: https://en.wikipedia.org/wiki/Selection_algorithm
	if (sources_use - 1 > hw_sources_num)
		std::nth_element(src_audible.begin(), src_audible.begin() + hw_sources_num,
						src_audible.begin() + sources_use-1, compareByAudibility);

	//  retire out of range sources first
	for (i=0; i < sources_use; i++)
		if (sources[src_audible[i].first]->hw_id != -1 && (i >= hw_sources_num || src_audible[i].second == 0))
			retire(src_audible[i].first);

	//  assign new sources
	for (i=0; i < std::min(sources_use, hw_sources_num); i++)
	if (sources[src_audible[i].first]->hw_id == -1 && src_audible[i].second > 0)
		for (int j=0; j < hw_sources_num; j++)
		if (hw_sources_map[j] == -1)
		{
			assign(src_audible[i].first, j);
			break;
		}
}


//  💫 recompute Source
//---------------------------------------------------------------------------------------------------------------------------
void SoundBaseMgr::recomputeSource(int id, int reason, float fl, Vector3* vec)
{
	if (!device)  return;
	sources[id]->computeAudibility(camera_position);

	if (sources[id]->audibility == 0.0f)
	{
		if (sources[id]->hw_id != -1)
			//  retire the source if it is currently assigned
			retire(id);
	}else
	{
		//  this is a potentially audible sources[id]
		if (sources[id]->hw_id != -1)
		{
			//  sources[id] already playing
			//  update the AL settings
			switch (reason)
			{
			case REASON_GAIN:  alSourcef(hw_sources[sources[id]->hw_id], AL_GAIN, fl * master_volume); break;
			case REASON_PTCH:  alSourcef(hw_sources[sources[id]->hw_id], AL_PITCH, fl); break;
			case REASON_POS:  if (!sources[id]->is2D)
							  alSource3f(hw_sources[sources[id]->hw_id], AL_POSITION, vec->x, vec->y, vec->z); break;
			case REASON_VEL:  alSource3f(hw_sources[sources[id]->hw_id], AL_VELOCITY, vec->x, vec->y, vec->z); break;

			case REASON_PLAY:  alSourcePlay(hw_sources[sources[id]->hw_id]); break;
			case REASON_STOP:  alSourceStop(hw_sources[sources[id]->hw_id]); break;
			case REASON_LOOP:  alSourcei(hw_sources[sources[id]->hw_id], AL_LOOPING, fl > 0.5f ? AL_TRUE : AL_FALSE); break;
			case REASON_SEEK:  alSourcei(hw_sources[sources[id]->hw_id], AL_SAMPLE_OFFSET, fl); break;
			default: break;
			}
		}else
		{
			//  try to make it play by the hardware
			//  check if there is one free sources[id] in the pool
			if (hw_sources_use < hw_sources_num)
			{
				for (int i=0; i < hw_sources_num; i++)
				{
					if (hw_sources_map[i] == -1)
					{
						assign(id, i);
						break;
					}
				}
			}else
			{
				//  now, compute who is the faintest
				//  note: we know hw_sources_map is full!
				float fv = 1.0f;
				int al_faintest = 0;
				for (int i=0; i < hw_sources_num; i++)
				{
					if (hw_sources_map[i] >= 0 && sources[hw_sources_map[i]]->audibility < fv)
					{
						fv = sources[hw_sources_map[i]]->audibility;
						al_faintest = i;
					}
				}
				//  check to ensure that the sound is louder than the faintest sound currently playing
				if (fv < sources[id]->audibility)
				{
					// this new sources[id] is louder than the faintest!
					retire(hw_sources_map[al_faintest]);
					assign(id, al_faintest);
				}
				//  else this sources[id] is too faint, we don't play it!
			}
		}
	}
}


//  assign
//-----------------------------------------------------------------------------------
void SoundBaseMgr::assign(int id, int hw_id)
{
	if (!device)  return;
	sources[id]->hw_id = hw_id;
	hw_sources_map[hw_id] = id;

	//  the hardware source is supposed to be stopped!
	ALuint source = hw_sources[hw_id];
	alSourcei(source, AL_BUFFER, sources[id]->buffer);

	//  use reverb +
	if (reverb)
		alSource3i(source, AL_AUXILIARY_SEND_FILTER,
			!sources[id]->is2D ? slot : AL_EFFECTSLOT_NULL,  0, AL_FILTER_NULL);

	alSourcef(source, AL_GAIN, sources[id]->gain * master_volume);
	alSourcei(source, AL_LOOPING, sources[id]->loop ? AL_TRUE : AL_FALSE);
	alSourcef(source, AL_PITCH, sources[id]->pitch);

	alSource3f(source, AL_POSITION, sources[id]->pos.x, sources[id]->pos.y, sources[id]->pos.z);
	alSource3f(source, AL_VELOCITY, sources[id]->vel.x, sources[id]->vel.y, sources[id]->vel.z);

	//  hud, 2d
	if (sources[id]->is2D)
	{
		alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
		alSource3f(source, AL_POSITION, 0.f,0.f,0.f);
		//alSourcef(source, AL_ROLLOFF_FACTOR, 0.f);
	}

	if (sources[id]->should_play)
		alSourcePlay(hw_sources[hw_id]);

	++hw_sources_use;
}


void SoundBaseMgr::retire(int id)
{
	if (!device)  return;
	//if (sources[id]->is2D)  return;
	if (sources[id]->hw_id == -1)  return;
	
	alSourceStop(hw_sources[sources[id]->hw_id]);
	hw_sources_map[sources[id]->hw_id] = -1;
	sources[id]->hw_id = -1;
	--hw_sources_use;
}


//  utility
void SoundBaseMgr::pauseAll(bool mute)
{
	if (!device)  return;
	alListenerf(AL_GAIN, mute ? 0.0f : master_volume);
}

void SoundBaseMgr::setMasterVolume(float vol)
{
	if (!device)  return;
	master_volume = vol;
	alListenerf(AL_GAIN, master_volume);
}


//  💫 Update
//-----------------------------------------------------------------------------------
void SoundBaseMgr::setCamera(Vector3 pos, Vector3 dir, Vector3 up, Vector3 vel)
{
	if (!device)  return;
	camera_position = pos;
	
	recomputeAllSources();

	float o[6];
	o[0] = dir.x;  o[1] = dir.y;  o[2] = dir.z;
	o[3] = up.x;  o[4] = up.y;  o[5] = up.z;

	alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
	alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);
	alListenerfv(AL_ORIENTATION, o);
}


void SoundBaseMgr::Update(float dt)
{
	//  ambient-
	// todo: cam vel.. doppler


	//  check all dynamic, remove finished
	int i = 0;
	auto it = dynamics.begin();
	while (it != dynamics.end())
	{
		if (!(*it)->isPlaying())
		{
			if ((*it)->pObj)
				(*it)->pObj->playing = 0;  // mark object as quiet

			// (*it)->stop();
			(*it)->Destroy();
			it = dynamics.erase(it);
		}
		else
		{
			(*it)->Update(dt);
			++it;
		}
		++i;
	}
	cnt_dynamic = i;
	// LogO(toStr(i));
}
