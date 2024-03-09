#include "pch.h"
#include "Def_Str.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"

#include <filesystem>
#include <Ogre.h>
#include <MyGUI_TextBox.h>
#include <SDL_keyboard.h>
#ifndef _WIN32
#include <dirent.h>
#endif
using namespace Ogre;
namespace fs = std::filesystem;


///  system file, dir
//-----------------------------------------------------------------------------------------------------------
bool CGui::Rename(String from, String to)
{
	try
	{	if (fs::exists(from.c_str()))
			fs::rename(from.c_str(), to.c_str());
	}
	catch (const fs::filesystem_error & ex)
	{
		String s = "Error: Renaming file " + from + " to " + to + " failed ! \n" + ex.what();
		strFSerrors += "\n" + s;
		LogO(s);
		return false;
	}
	return true;
}

bool CGui::Delete(String file)
{
	try
	{	if (fs::exists(file.c_str()))
			fs::remove(file.c_str());
	}
	catch (const fs::filesystem_error & ex)
	{
		String s = "Error: Deleting file " + file + " failed ! \n" + ex.what();
		strFSerrors += "\n" + s;
		LogO(s);
		return false;
	}
	return true;
}

bool CGui::DeleteDir(String dir)
{
	try
	{	fs::remove_all(dir.c_str());
	}
	catch (const fs::filesystem_error & ex)
	{
		String s = "Error: Deleting directory " + dir + " failed ! \n" + ex.what();
		strFSerrors += "\n" + s;
		LogO(s);
		return false;
	}
	return true;
}

bool CGui::CreateDir(String dir)
{
	try
	{	fs::create_directory(dir.c_str());
	}
	catch (const fs::filesystem_error & ex)
	{
		String s = "Error: Creating directory " + dir + " failed ! \n" + ex.what();
		strFSerrors += "\n" + s;
		LogO(s);
		return false;
	}
	return true;
}

bool CGui::Copy(String file, String to)
{
	try
	{	if (fs::exists(to.c_str()))
			fs::remove(to.c_str());

		if (fs::exists(file.c_str()))
			fs::copy_file(file.c_str(), to.c_str());
	}
	catch (const fs::filesystem_error & ex)
	{
		String s = "Error: Copying file " + file + " to " + to + " failed ! \n" + ex.what();
		strFSerrors += "\n" + s;
		LogO(s);
		return false;
	}
	return true;
}


void App::UpdWndTitle()
{
	String s = String("SR Editor  track: ") + pSet->gui.track;
	if (pSet->gui.track_user)  s += "  *user*";

	SDL_SetWindowTitle(mSDLWindow, s.c_str());  // fixme?
}


//  keys, mouse btn  input info bar  ====================================
void App::UpdKeyBar(Real dt)
{
	const int Kmax = SDL_SCANCODE_SLEEP;  // last key
	static float tkey[Kmax+1] = {0.f,};  // key delay time
	int i;
	static bool first = true;
	if (first)
	{	first = false;
		for (i = Kmax; i > 0; --i)  tkey[i] = 0.f;
	}
	String ss = "   ";
	//  pressed
	for (i = Kmax; i > 0; --i)
		if (IsKey(SDL_Scancode(i)))
			tkey[i] = 0.2f;  // par  min time to display

	//  modifiers
	const static int
		lc = SDL_SCANCODE_LCTRL,  rc = SDL_SCANCODE_RCTRL,
		la = SDL_SCANCODE_LALT,   ra = SDL_SCANCODE_RALT,
		ls = SDL_SCANCODE_LSHIFT, rs = SDL_SCANCODE_RSHIFT;

	if (tkey[lc] > 0.f || tkey[rc] > 0.f)	ss += "#D0F0D0Ctrl  ";
	if (tkey[la] > 0.f || tkey[ra] > 0.f)	ss += "#D0F0D0Alt  ";
	if (tkey[ls] > 0.f || tkey[rs] > 0.f)	ss += "#D0F0D0Shift  ";

	//  mouse buttons
	if (mbLeft)  ss += "#C0FFFFLMB  ";
	if (mbRight)  ss += "#C0FFFFRMB  ";
	if (mbMiddle)  ss += "#C0FFFFMMB  ";

	//  all
	for (i=Kmax; i > 0; --i)
	{
		if (tkey[i] > 0.f)
		{	tkey[i] -= dt;  // dec time
			if (i!=lc && i!=la && i!=ls && i!=rc && i!=ra && i!=rs)
			{
				String s = String(SDL_GetKeyName(SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(i))));
				s = StringUtil::replaceAll(s, "Keypad", "#FFFFC0Num ");
				ss += "#FFFFFF" + s + "  ";
			}
	}	}
	
	//  mouse wheel
	static int mzd = 0;
	if (mz > 0)  mzd = 30;
	if (mz < 0)  mzd = -30;
	if (mzd > 0)  {  ss += "#D0D8FFWheel up";  --mzd;  }
	if (mzd < 0)  {  ss += "#D0D8FFWheel down";  ++mzd;  }
	txInput->setCaption(ss);
}
