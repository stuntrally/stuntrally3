#pragma once
#include <vector>
#include <set>

#include <Ogre.h>
#include <OgreMesh.h>

namespace Ogre {  class SceneManager;  class SceneNode;  class Item;  class Terra;  }


// #ifdef SR_EDITOR
// #define LogR(a)  //LogO(String("~ Road  ") + a)
// #else
// #define LogR(a)  //LogO(String("~ Road  ") + a)
// #endif

// #define  LODs  4  // LODs each segment has
// #define  MTRs  4  // road materials to choose


struct GrassData  // SegData
{
	//  ogre resources for 1 segment
	Ogre::SceneNode* node =0;  Ogre::Item* it =0;
	Ogre::String smesh;
	// Ogre::MeshPtr mesh;  Ogre::v1::MeshPtr mesh1;
};

#if 0
struct GrassPage  // RoadSeg
{
	GrassData road[LODs];
	Ogre::String sMtrRd;
	int mtrId = 0;
	
	// std::vector<Ogre::Vector3> lpos;  //points for lod dist
	// int nTri[LODs], mrgLod = 0;
	bool empty = true;

	// GrassPage()
	// {	for (int i=0; i<LODs; ++i)  nTri[i] = 0;  }
};
#endif

class Grass
{
public:
	// Grass();
	// void Defaults();

	Ogre::SceneManager* mSceneMgr = 0;
	Ogre::Terra* terrain = 0;
	class CScene* scn = 0;
	class SETTINGS* pSet = 0;

	
	//  File
	// bool LoadFile(Ogre::String fname, bool build = true), SaveFile(Ogre::String fname);
	
	//  Rebuild
	// bool RebuildRoadInt(bool editorAlign = false, bool edBulletFull = false);
	// void RebuildRoadPace();  ///  Rebuild road only for pacenotes, after RebuildRoadInt
	void Create();
	void Destroy();//, DestroySeg(int id);


	//  Update
	// void UpdLodVis(float fBias = 1.f, bool bFull = false);
	// void SetForRnd(Ogre::String sMtr), UnsetForRnd();



private:
///  ***  MESH  ****
//---------------------------------------------------------------------------------------

	std::vector<GrassData> gds;

	void CreateMesh( GrassData& sd, Ogre::String sMesh, Ogre::String sMtrName,
		const std::vector<Ogre::Vector3>& pos, const std::vector<Ogre::Vector3>& norm,
		const std::vector<Ogre::Vector4>& clr, const std::vector<Ogre::Vector2>& tcs,
		const std::vector<Ogre::uint16>& idx);

	std::vector<Ogre::uint16>    idx;	  // mesh indices

	//  add triangle, with index check
	void addTri(int f1, int f2, int f3, int i);

	// int at_size, at_ilBt;
	// bool bltTri, blendTri;  // params for addTri
	
	
///  ***  Rebuild Geom DATA  ***
//---------------------------------------------------------------------------------------
#if 0	
	struct DataRoad  // global
	{
		int segs = 0;            // count
		int sMin = 0, sMax = 0;  // range
		
		DataRoad(bool edAlign, bool bltFull)
		{	}
	};


	struct DataLod0   // at Lod 0
	{
		std::vector<int>            v0_iL;  // length steps
		std::vector<Ogre::Real>     v0_tc;  // tex coords
		std::vector<Ogre::Vector3>  v0_N;   // normals
		std::vector<int>          v0_Loop;  // bool, inside loop
		void Clear()
		{	v0_iL.clear();  v0_tc.clear();  v0_N.clear();  v0_Loop.clear();  }
	}
	DL0;  // stays after build since N is used for SetChecks
	void SetChecks();  // Init  1st in file load, 2nd time for N


	struct DataLod   // for current Lod
	{
		//>  data at cur lod
		std::vector<int>  v_iL, v_iW;  // num Length and Width steps for each seg
		std::vector<int>  v_bMerge;    // bool 0 if seg merged, 1 if new
		
		std::vector<Ogre::Real>     v_tc, v_len;  // total length
		std::vector<Ogre::Vector3>  v_W;   // width dir
		
		std::vector<std::vector <int> >  v_iWL;  //  width steps per length point, for each seg
		std::vector<int>  v_iwEq;	   // 1 if equal width steps at whole length, in seg, 0 has transition

		Ogre::Real tcLen = 0.f;      // total tex coord length u
		Ogre::Real sumLenMrg = 0.f;  // total length to determine merging
		int mrgCnt = 0;              // stats, merges counter

		//  LOD vars
		int lod = 0, iLodDiv = 1;  //.
		Ogre::Real fLenDim = 1.f;
		bool isLod0 = true, isPace = false;
	};
	
	struct StatsLod   // stats for current Lod
	{	//#  stats
		Ogre::Real roadLen = 0.f, rdOnT = 0.f, rdPipe = 0.f, rdOnPipe = 0.f;
		Ogre::Real avgWidth = 0.f, stMaxH = -30000.f, stMinH = 30000.f;
		Ogre::Real bankAvg = 0.f, bankMax = 0.f;
		bool stats = false;
	};
	
	void PrepassLod(
		const DataRoad& DR,
		DataLod0& DL0, DataLod& DL, StatsLod& ST,
		int lod, bool editorAlign);


	struct DataLodMesh   // mesh data for lod  (from merged segs)
	{
		//>  W-wall  C-column  B-blend
		std::vector<Ogre::Vector4>  clr0/*empty*/, clr, clrB;
		std::vector<Ogre::Vector3>  pos,norm, posW,normW, posC,normC, posLod, posB,normB;
		std::vector<Ogre::Vector2>  tcs, tcsW, tcsC, tcsB;

		int iLmrg = 0, iLmrgW = 0, iLmrgC = 0, iLmrgB = 0;
		
		void Clear();
	};
#endif
	
//---------------------------------------------------------------------------------------


//  vars
	// friend class App;
	// friend class CGui;
public:
	// Ogre::Vector3 posHit;  bool bHitTer = false;
	
	// int iOldHide = -1, idStr = 0;  // upd var

	// bool bMerge;
	// float fLodBias = 1.f;     // upd par, detail
	
	// bool bCastShadow = 0;    // true for depth shadows
	// bool bRoadWFullCol = 0;  // road wall full collision (all triangles, or just side)


	//  road data Segments
	// std::deque<GrassPage> vSegs;


///  params, from xml
	//  materials
	// Ogre::String  sMtrPipe[MTRs];  // use SetMtrPipe to set
	// bool bMtrPipeGlass[MTRs];  // glass in mtr name

	// Ogre::String  sMtrRoad[MTRs], sMtrWall,sMtrWallPipe, sMtrCol;
	// bool bMtrRoadTer[MTRs];  // if _ter material present in .mat
	// void updMtrRoadTer();
	// void SetMtrPipe(int i, Ogre::String sMtr);


	//  geometry  ----
	//  tex coord multipliers (scale) per unit length
			//  road, wall, pipe, pipewall, column
	// Ogre::Real g_tcMul, g_tcMulW,  g_tcMulP, g_tcMulPW,  g_tcMulC;

	// Ogre::Real g_LenDim0;	// triangle dim in length
	// int  g_iWidthDiv0;		// width divisions (const for road, except pipes)

	//  merge  for less batches
	// Ogre::Real g_MergeLen;     // length below which segments are merged
	// Ogre::Real g_LodPntLen;    // length between LOD points

	Ogre::Real g_VisDist;      // max visible distance
	Ogre::Real g_VisBehind;    // distance visible behind, meh


	//  stats  ----
	/*struct Stats  // for info only
	{
		int iMrgSegs = 0, segsMrg = 0;
		int iVis = 0, iTris = 0;  // in upd vis

		Ogre::Real Length = 0.f, WidthAvg = 0.f, HeightDiff = 0.f;
		
		void Reset();
	} st;*/

	// void End0Stats(const DataLod& DL, const StatsLod& ST);
	// void EndStats(const DataRoad& DR, const StatsLod& ST);
};
