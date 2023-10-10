#pragma once
#include <vector>
#include <set>

#include <OgreString.h>
#include <OgreVector2.h>
#include <OgreVector3.h>
#include <OgreVector4.h>

namespace Ogre {  class SceneManager;  class SceneNode;  class Item;  class Terra;  }
class App;


struct GrassData  // SegData
{
	//  🟢 Ogre  1 page
	Ogre::SceneNode* node =0;  Ogre::Item* it =0;
	Ogre::String smesh;
	//Ogre::MeshPtr mesh;  Ogre::v1::MeshPtr mesh1;
};

/*struct GrassPage  // RoadSeg
{
	GrassData road[LODs];
	Ogre::String sMtrRd;
	int mtrId = 0;
	bool empty = true;
};*/


class Grass  //  🌿  ------------------
{
public:
	App* app =0;

	Ogre::SceneManager* mSceneMgr = 0;
	Ogre::Terra* terrain = 0;
	class CScene* scn = 0;
	class SETTINGS* pSet = 0;

	void Create(App* app1);
	void Destroy();


private:
	//  Mesh
	std::vector<GrassData> gds;

	void CreateMesh( GrassData& sd, Ogre::String sMesh, Ogre::String sMtrName,
		const std::vector<Ogre::Vector3>& pos, const std::vector<Ogre::Vector3>& norm,
		const std::vector<Ogre::Vector4>& clr, const std::vector<Ogre::Vector2>& tcs,
		const std::vector<Ogre::uint32>& idx);

	std::vector<Ogre::uint32> idx;  // mesh indices

	//  add triangle, with index check
	void addTri(int f1, int f2, int f3, int i);
};
