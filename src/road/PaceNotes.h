#pragma once
#include "HudRenderable.h"
#include <vector>
// #include <Ogre.h>
#include <OgreVector2.h>
#include <OgreVector3.h>
#include <OgreVector4.h>

namespace Ogre {  class SceneManager;  class SceneNode;  class Terra;  class Camera;  class Window;
	namespace v1  {  class BillboardSet;  class Billboard;  }  }
namespace MyGUI {  class TextBox;  class Gui;  }
class SplineRoad;  class Scene;  class SETTINGS;
struct Cam;

#if 0
enum PaceTypes                 // 90     // 180    // 270
{	P_1=0, P_2, P_3, P_4, P_5, P_6sq, P_7hrp, P_8u, P_9o,  // turns
	P_Loop, P_LoopBig, P_LoopSide, P_LoopBarrel,  // loops
	P_Jump, P_JumpTer,  // jumps
	P_OnPipe,
	P_Bumps,
	//  manual..
	P_Slow, P_Stop, P_Danger,   // brake, warn
	P_Obstacle, P_Narrow, P_Split,
	P_Ice, P_Mud, P_Water,  //..
	Pace_ALL
};
#endif

struct PaceNote  // 🚦 one
{
	bool vis = false;  // visible in game, set in UpdVis
	MyGUI::TextBox* txt =0;  // text for jmp vel

	//  data
	Ogre::Vector3 pos;
	Ogre::Vector4 clr;
	Ogre::Vector2 size, ofs,uv;
	int use = 1;  // 1 normal  2 dbg start  3 dbg cont  4 bar  5 trk gho
	int id = 0;
	bool start = 0;  // start pos only
	int jump = 0;  // 0 none 1 jump 2 land

	//PaceTypes type;
	//int dir;  // -1 left, 1 right
	float vel = 0.f;  // for jump
	bool text = 0;

	// PaceNote();
	PaceNote(int i,int t, Ogre::Vector3 p, float sx,float sy,
		float r,float g,float b,float a,
		float ox,float oy, float u,float v);
};


class PaceNotes  // 🚦 All
{
public:
	SETTINGS* pSet =0;  ///*
	PaceNotes(SETTINGS* pset);
	~PaceNotes();
	//void Defaults();

	//  🌟 Setup, call this on Init
	void Setup(Ogre::SceneManager* sceneMgr, Cam* camera,
		Ogre::Terra* terrain, MyGUI::Gui* gui, Ogre::Window* window);
	void SetupTer(Ogre::Terra* terrain);

	//  🆕 Rebuild
	void Rebuild(SplineRoad* road, Scene* sc, bool reversed);
	void CreateHR();
	void Destroy(), Destroy(PaceNote& n);
	void Create(PaceNote& n);
	void Update(PaceNote& n), UpdateTxt(PaceNote& n), UpdTxt();
	void Reset();
	
	//  💫 Update
	void UpdVis(Ogre::Vector3 carPos, bool hide=false);
	void updTxt(PaceNote& n, bool vis);

private:
	//  🟢 ogre vars
	Ogre::SceneManager* mSceneMgr =0;
	Ogre::Terra* mTerrain =0;
	MyGUI::Gui* mGui =0;
	Ogre::Window* mWindow =0;
	Cam* cam =0;
	
	//  all notes
	HudRenderable* hr =0;
	Ogre::SceneNode* ndHR =0;
	int count = 4;  //par
	
	std::vector<PaceNote>
		vPN,  // all incl. debug
		vPS;  // game, signs only, sorted by id
	int ii = 0;  // id for names
public:
	int iStart = 0;  // vPS id of track start
	int iAll = 1;  // all road markers from road->vPace
	int iDir = 1;  // copy from road
	int iCur = 0;  // cur car pace id, for tracking
	float carVel = 140.f/3.6f;  // for ed
	bool rewind = 0;
};


static bool PaceSort(const PaceNote& a, const PaceNote& b)
{
	return a.id < b.id;
}
