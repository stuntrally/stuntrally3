#include "pch.h"
#include "Def_Str.h"
#include "SoundMgr.h"
#include "SoundBase.h"
#include "SoundDynamic.h"
#include "SoundBaseMgr.h"
#include <OgreDataStream.h>
#include <OgreException.h>
using namespace Ogre;


//  Init
//---------------------------------------------------------------------------------------
SoundMgr::SoundMgr(SETTINGS* pSet1)
	:disabled(true), sound_mgr(0), pSet(pSet1)
{	}

bool SoundMgr::Init(std::string snd_device, bool reverb)
{
	sound_mgr = new SoundBaseMgr(pSet);

	bool ok = sound_mgr->Init(snd_device, reverb);
	if (!ok)
	{	LogO("@  SoundScript: Failed to create Sound Manager");  return false;  }

	disabled = sound_mgr->isDisabled();
	if (disabled)
	{	LogO("@  SoundScript: Sound Manager is disabled");  return false;  }

	LogO("@  SoundScript: Sound Manager started with " + toStr(sound_mgr->hw_sources_num)+" sources");
	return true;
}

SoundMgr::~SoundMgr()
{
	for (size_t i=0; i < v_templ.size(); ++i)
		delete v_templ[i];
	delete sound_mgr;
}

//  create template
//---------------------------------------------------------------------------------------
SoundTemplate* SoundMgr::createTemplate(String name, String filename)
{
	// first, search if there is a template name collision
	if (templates.find(name) != templates.end())
		OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, "SoundScript with the name " + name + " already exists.", "SoundScriptManager::createTemplate");

	SoundTemplate *ssi = new SoundTemplate(name, filename);
	templates[name] = ssi;
	v_templ.push_back(ssi);
	return ssi;
}


//  create Sound
//---------------------------------------------------------------------------------------
Sound* SoundMgr::createInstance(String name)
{
	//  search template
	if (templates.find(name) == templates.end())
	{
		LogO("@ sound template not found in .cfg: "+name);
		return NULL;
	}

	SoundTemplate* templ = templates[name];

	Sound* inst = new Sound(templ, sound_mgr);
	
	String ss = name.substr(0,4);
	inst->set2D(ss=="hud/");  // set 2d

	//  start looped
	if (!inst->start_sound)
	{	inst->setGain(0.f);
		inst->start();  // 🔉
	}
	return inst;
}


//  Parse
//---------------------------------------------------------------------------------------
void SoundMgr::parseScript(FileStreamDataStream* stream)
{
	SoundTemplate* sst = 0;
	String line = "";  int cnt=0;

	LogO("@  SoundScript: Parsing"/*+stream->getName()*/);

	while (!stream->eof())
	{
		line = stream->getLine();
		//  ignore comments & blanks
		if (!(line.length() == 0 || line.substr(0,2) == "//"))
		{
			if (sst == 0)
			{
				//  no current SoundScript
				//  so first valid data should be a SoundScript name
				//LogO("@  SoundScriptManager: creating template "+line);
				//LogO("@  "+line);
				sst = createTemplate(line, stream->getName());
				if (!sst)
				{
					//  there is a name collision for this Sound Script
					LogO("@  Warning: sound already defined: "+line);
					skipToNextOpenBrace(stream);
					skipToNextCloseBrace(stream);
					continue;
				}
				skipToNextOpenBrace(stream);  ++cnt;
			}else
			{
				//  already in a ss
				if (line == "}")
					sst = 0;  // finished ss
				else
				{	//  attribute
					//  split params on space
					StringVector veclineparams = StringUtil::split(line, "\t ", 0);

					if (!sst->setParameter(veclineparams))
						LogO("@  Bad SoundScript attribute line: '"+line);
				}
			}
	}	}
	LogO("@  SoundScript: Parsed: "+toStr(cnt)+" templates.");
}

//  this requires new lines..
void SoundMgr::skipToNextCloseBrace(FileStreamDataStream* stream)
{
	String line = "";

	while (!stream->eof() && line != "}")
	{
		line = stream->getLine();
	}
}

void SoundMgr::skipToNextOpenBrace(FileStreamDataStream* stream)
{
	String line = "";

	while (!stream->eof() && line != "{")
	{
		line = stream->getLine();
	}
}


//  Load script
//---------------------------------------------------------------------------------------
SoundTemplate::SoundTemplate(String name1, String filename1)
	:file_name(filename1), name(name1), free_sound(0)
	,has_start(false), has_stop(false)
	,unpitchable(false)
{
}

bool SoundTemplate::setParameter(StringVector vec)
{
	if (vec.empty())  return false;

	if (vec[0] == "start")
	{
		if (vec.size() < 2)  return false;
		start_name  = vec[1];
		has_start   = true;
		return true;
	}
	else
	if (vec[0] == "stop")
	{
		if (vec.size() < 2)  return false;
		stop_name  = vec[1];
		has_stop   = true;
		return true;
	}
	else
	if (vec[0] == "sound")
	{
		if (vec.size() < 3)  return false;
		if (free_sound >= MAX_SOUNDS_PER_SCRIPT)
		{
			LogO("@  Reached MAX_SOUNDS_PER_SCRIPT limit (" + toStr(MAX_SOUNDS_PER_SCRIPT) + ")");
			return false;
		}
		sound_pitches[free_sound] = StringConverter::parseReal(vec[1]);  // unpitched = 0.0
		if (sound_pitches[free_sound] == 0)
			unpitchable = true;

		if (free_sound > 0 && !unpitchable && sound_pitches[free_sound] <= sound_pitches[free_sound - 1])
			return false;

		sound_names[free_sound] = vec[2];
		++free_sound;
		return true;
	}
	return false;
}


///  Sound ctor
//---------------------------------------------------------------------------------------------------------
int Sound::instances = 0;

Sound::Sound(SoundTemplate* tpl, SoundBaseMgr* mgr1)
	:templ(tpl), sound_mgr(mgr1)
	,start_sound(0), stop_sound(0)
	,lastgain(1.0f), is2D(false), engine(false)
{
	++instances;
	//  create sounds
	if (tpl->has_start)
		start_sound = sound_mgr->createSound(tpl->start_name, templ->name);
	
	if (tpl->has_stop)
		stop_sound = sound_mgr->createSound(tpl->stop_name, templ->name);
	
	for (int i=0; i < tpl->free_sound; i++)
		sounds[i] = sound_mgr->createSound(tpl->sound_names[i], templ->name);
	
	//if (tpl->free_sound > 0)
		setPitch(1.f);
	setGain(0.f);

	//LogO("@  Sound created: "+name+" "+toStr(tpl->free_sound));
}

Sound::~Sound()
{
	--instances;
	delete start_sound;
	delete stop_sound;
	
	if (templ)
	for (int i=0; i < templ->free_sound; i++)
		delete sounds[i];
}


//  New ambient
//---------------------------------------------------------------------
void SoundMgr::CreateAmbient(String name)
{
	if (sound_mgr->ambient)
		DestroyAmbient();

	//  search template
	if (templates.find(name) == templates.end())
	{
		LogO("@ sound ambient template not found in .cfg: "+name);
		return;
	}
	LogO("@ sound ambient create: "+name);

	SoundTemplate* templ = templates[name];

	sound_mgr->ambient = new SoundDynamic(sound_mgr);
	sound_mgr->ambient->Create(templ->sound_names[0], true, true, 0);
}

void SoundMgr::DestroyAmbient()
{
	if (!sound_mgr->ambient)
		return;
	sound_mgr->ambient->stop();
	sound_mgr->ambient->Destroy();
	
	delete sound_mgr->ambient;
	sound_mgr->ambient = 0;
}
