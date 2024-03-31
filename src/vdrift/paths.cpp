#include "pch.h"
/// Big portions of this file are borrowed and adapted from Performous under GPL (https://performous.org)

#include "paths.h"
#include <filesystem>
#include <string>
#include <fstream>
#include <list>
#include <cassert>
#include <sstream>
#include <iostream>

#ifdef _WIN32
	#include <windows.h>
	#include <shlobj.h>
	#include <shellapi.h>
#else
	#include <sys/types.h>
	#include <dirent.h>
#endif
//#include "Def_Str.h"  // LogO

// Should come from CMake
#ifndef SHARED_DATA_DIR
#define SHARED_DATA_DIR "data"
#endif
using namespace std;

namespace fs = std::filesystem;

namespace
{
	fs::path execname();
}


//  static vars
string /*PATHS::ogre_plugin,*/ PATHS::home_dir,
	PATHS::user_config, PATHS::game_config,
	PATHS::user_data, PATHS::game_data,
	PATHS::cache_dir, PATHS::game_bin;
stringstream PATHS::info;


void PATHS::Init(bool log_paths)
{
	typedef vector<fs::path> Paths;

	// Set Ogre plugins dir
/*	{
		ogre_plugin = "";
		char *plugindir = getenv("OGRE_PLUGIN_DIR");
		if (plugindir) {
			ogre_plugin = plugindir;
		} else {
			#ifdef _WIN32
			ogre_plugin = ".";
			#else
			//; ogre_plugin = OGRE_PLUGIN_DIR_REL;
			#endif
		}
	}
	//ogre_plugin = "/usr/lib/x86_64-linux-gnu/OGRE-1.9.0";
	ogre_plugin = "/usr/local/lib/OGRE";
*/

	fs::path stuntrally3 = "stuntrally3";
	// Figure out the user's home directory
	{
		home_dir = "";
		#ifndef _WIN32 // POSIX
			char *homedir = getenv("HOME");
			if (homedir == NULL)
			{
				home_dir = "/home/";
				homedir = getenv("USER");
				if (homedir == NULL) {
					homedir = getenv("USERNAME");
					if (homedir == NULL) {
						cerr << "Could not find user's home directory!" << endl;
						home_dir = "/tmp/";
					}
				}
			}
		#else // Windows
			char *homedir = getenv("USERPROFILE");
			if (homedir == NULL) homedir = "data"; // WIN 9x/Me
		#endif
		home_dir += homedir;
	}

	// Find user's config dir
	#ifndef _WIN32 // POSIX
	{
		char const* conf = getenv("XDG_CONFIG_HOME");
		if (conf) user_config = (fs::path(conf) / stuntrally3).string();
		else user_config = (fs::path(home_dir) / ".config" / stuntrally3).string();
	}
	#else // Windows
	{
		// Open AppData directory
		string str;
		ITEMIDLIST* pidl;
		char AppDir[MAX_PATH];
		HRESULT hRes = SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE, &pidl);
		if (hRes == NOERROR)
		{
			SHGetPathFromIDListA(pidl, AppDir);
			int i;
			for (i = 0; AppDir[i] != '\0'; ++i) {
				if (AppDir[i] == '\\') str += '/';
				else str += AppDir[i];
			}
			user_config = (fs::path(str) / stuntrally3).string();
		}
	}
	#endif
	// Create user's config dir
	CreateDir(user_config);

	// Find user's data dir (for additional data)
	#ifdef _WIN32
	user_data = user_config;  // APPDATA/stuntrally3
	#else
	{
		char const* xdg_data_home = getenv("XDG_DATA_HOME");
		user_data = (xdg_data_home ? xdg_data_home / stuntrally3
					: fs::path(home_dir) / ".local/share" / stuntrally3).string();
	}
	#endif

	// Create user's data dir and its children
	///--------------------------------------------------
	CreateDir(user_data);
	CreateDir(Records());
	CreateDir(Ghosts());
	
	CreateDir(Replays());
	CreateDir(Screenshots());
	CreateDir(TracksUser());  // user tracks

	CreateDir(DataUser());  // user data

	
	fs::path exe = execname();  // binary dir
	game_bin = exe.parent_path();


	// Find game data dir and defaults config dir
	char *datadir = getenv("STUNTRALLY3_DATA_ROOT");
	if (datadir)
		game_data = string(datadir);
	else
	{	fs::path shareDir = SHARED_DATA_DIR;
		Paths dirs;

		// todo: Adding users data dir?
		//dirs.push_back(user_data_dir);

		// Adding relative path for running from sources
		dirs.push_back(exe.parent_path().parent_path().parent_path() / "data");
		dirs.push_back(exe.parent_path().parent_path().parent_path());
		dirs.push_back(exe.parent_path().parent_path() / "data");
		dirs.push_back(exe.parent_path().parent_path());
		dirs.push_back(exe.parent_path() / "data");
		dirs.push_back(exe.parent_path());
		// Adding relative path from installed executable
		dirs.push_back(exe.parent_path().parent_path() / shareDir);
		#ifndef _WIN32
		// Adding XDG_DATA_DIRS
		{
			char const* xdg_data_dirs = getenv("XDG_DATA_DIRS");
			istringstream iss(xdg_data_dirs ? xdg_data_dirs : "/usr/local/share/:/usr/share/");
			for (string p; getline(iss, p, ':'); dirs.push_back(p / stuntrally3)) {}
		}
		#endif
		// TODO: Adding path from config file

		//  Loop through the paths and pick the first one that contain some data
		for (const auto& d : dirs)
		{
			//  Data dir
			// LogO(d);
			if (fs::exists(d / "sounds"))
				game_data = d.string();
			//  Config dir
			if (fs::exists(d / "config"))
				game_config = (d / "config").string();

			//  Check if both are found
			if (!game_data.empty() && !game_config.empty())  break;
		}
	}


	//  Subdirs for each sim_mode
	///--------------------------------------------------
	list <string> li;
	PATHS::DirList(PATHS::CarSim(), li);
	for (const auto& d : li)
	{
		CreateDir(Records()+"/"+d);
		CreateDir(Ghosts()+"/"+d);
	}

	//  Find cache dir
	#ifdef _WIN32
	cache_dir = user_config + "/cache";  // APPDATA/stuntrally3/cache
	#else
	char const* xdg_cache_home = getenv("XDG_CACHE_HOME");
	cache_dir = (xdg_cache_home ? xdg_cache_home / stuntrally3
				: fs::path(home_dir) / ".cache" / stuntrally3).string();
	#endif

	//  Create cache dir
	CreateDir(CacheDir());
	CreateDir(ShadersDir());
	CreateDir(MaterialsDir());

	//  Print diagnostic info
	if (log_paths)
	{
		info.str(string());  // clear, to not have twice
		info << "Paths info" << endl;
		info << "-------------" << endl;
		// info << "Ogre plugin-: " << ogre_plugin << endl;
		info << "Bin exe:      " << Bin() << endl;
		info << "Data:         " << Data() << endl;
		info << "ConfigDir:    " << GameConfigDir() << endl;  //?
		// info << "Home:         " << home_dir << endl;
		info << "User cfg,log: " << UserConfigDir() << endl;
		info << "User data:    " << user_data << endl;
		info << "Cache:        " << CacheDir() << endl;
		info << "-------------";
	}
}


//  utils
bool PATHS::FileExists(const string& filename)
{
	return fs::exists(filename);
}

bool PATHS::CreateDir(const string& path)
{
	try	{	fs::create_directories(path);	}
	catch (...)
	{
		cerr << "Could not create directory " << path << endl;
		return false;
	}
	return true;
}


bool PATHS::DirList(string dirpath, strlist& dirlist, string extension)
{
//------Folder listing code for POSIX
#ifndef _WIN32
	DIR *dp;
	struct dirent *ep;
	dp = opendir(dirpath.c_str());
	if (dp != NULL)
	{
		while (ep = readdir(dp))
		{
			//puts (ep->d_name);
			string newname = ep->d_name;
			if (newname[0] != '.')
			{
				dirlist.push_back(newname);
			}
		}
		(void) closedir(dp);
	}
	else
		return false;
#else
//------Folder listing for WIN32
	HANDLE          hList;
	CHAR            szDir[MAX_PATH+1];
	WIN32_FIND_DATAA FileData;

	// Get the proper directory path
	sprintf(szDir, "%s\\*", dirpath.c_str());

	// Get the first file
	hList = FindFirstFileA(szDir, &FileData);
	if (hList == INVALID_HANDLE_VALUE)
	{ 
		//no files found.  that's OK
	}
	else
	{
		// Traverse through the directory structure
		while (FindNextFileA(hList, &FileData))
		{
			// Check the object is a directory or not
			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
			{} else
			{
				if (FileData.cFileName[0] != '.')
				{
					dirlist.push_back(FileData.cFileName);
				}
			}
		}
	}
	FindClose(hList);
#endif
//------End
	
	// remove non-matcthing extensions
	if (!extension.empty())
	{
		list <list <string>::iterator> todel;
		for (auto it = dirlist.begin(); it != dirlist.end(); ++it)
		{
			if (it->find(extension) != it->length()-extension.length())
				todel.push_back(it);
		}
		
		for (auto it = todel.begin(); it != todel.end(); ++it)
			dirlist.erase(*it);
	}
	
	dirlist.sort();
	return true;
}


namespace
{
	/// Get the current executable name with path. Returns empty path if the name
	/// cannot be found. May return absolute or relative paths.
#if defined(_WIN32)
	#include <windows.h>
	fs::path execname()
	{
		char buf[1024];
		DWORD ret = GetModuleFileNameA(NULL, buf, sizeof(buf));
		if (ret == 0 || ret == sizeof(buf)) return fs::path();
		return buf;
	}

#elif defined(__APPLE__)
	#include <mach-o/dyld.h>
	fs::path execname()
	{
		char buf[1024];
		uint32_t size = sizeof(buf);
		int ret = _NSGetExecutablePath(buf, &size);
		if (ret != 0) return fs::path();
		return buf;
	}

#elif defined(sun) || defined(__sun)
	#include <stdlib.h>
	fs::path execname()
	{
		return getexecname();
	}

#elif defined(__FreeBSD__)
	#include <sys/sysctl.h>
	fs::path execname()
	{
		int mib[4];
		mib[0] = CTL_KERN;
		mib[1] = KERN_PROC;
		mib[2] = KERN_PROC_PATHNAME;
		mib[3] = -1;
		char buf[1024];
		size_t maxchars = sizeof(buf) - 1;
		size_t size = maxchars;
		sysctl(mib, 4, buf, &size, NULL, 0);
		if (size == 0 || size >= maxchars) return fs::path();
		buf[size] = '\0';
		return buf;
	}
	
#elif defined(__linux__)
	#include <unistd.h>
	fs::path execname()
	{
		char buf[1024];
		ssize_t maxchars = sizeof(buf) - 1;
		ssize_t size = readlink("/proc/self/exe", buf, sizeof(buf));
		if (size <= 0 || size >= maxchars) return fs::path();
		buf[size] = '\0';
		return buf;
	}
#else
	fs::path execname()
	{
		return fs::path();
	}
#endif
}


//  Open web browser with Url
void PATHS::OpenUrl(const string& url)
{
#ifdef WIN32
	ShellExecuteA(0, 0, url.c_str(), 0, 0 , SW_SHOW);
#else
	string cmd = "xdg-open " + url;
	system(cmd.c_str());
#endif
}

//  Start binary, exe  ----
//  Crucial for starting  Track Editor
//  from  Linux .AppImage pack  which starts game only
void PATHS::OpenBin(const string& cmd, const string& path)
{
#ifdef WIN32
	ShellExecuteA(
		0,  // hwnd
		0,  // str operation
		cmd.c_str(),  // file
		0,  // params
		path.c_str(),  // dir
		SW_SHOW);
#else
	system(cmd.c_str());  // no path, uses pwd?
#endif
}
