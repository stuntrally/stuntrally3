#pragma once
#include "Road.h"
#include <vector>
#include <OgreVector3.h>
#include <OgreVector4.h>
#include <OgreVector4.h>
namespace Ogre
{	class SceneManager;  class SceneNode;  class Item;  }
class App;


typedef std::tuple<short,short,short> GridPos;

struct GridMtr
{
	Ogre::String name;  // ogre datablock / material name
	// bool alpha;
	// bool pipeGlass;
	
};

static bool operator<(const GridMtr& a, const GridMtr& b)
{
	return a.name < b.name;
}


//  🏗️ 1 lod mesh
//---------------------------------------
struct GridCellMesh  // data
{
	//  one mtr  1 seg mesh
	//  any of: road, pipe,  wall column,  blend road
	//Ogre::String mtr;

	std::vector<Ogre::Vector3>  pos, norm;
	std::vector<Ogre::Vector2>  tcs;
	// std::vector<Ogre::Real>  clr;  // alpha only..
	std::vector<Ogre::Vector4>  clr;
	std::vector<Ogre::uint16>  idx;

	void Clear();
};


//  🟢 grid cell  with all lods meshes
//---------------------------------------
struct GridCellLods  // ogre mesh
{
	GridCellMesh lods[LODs];
	GridMtr mtr;

	static App* pApp;  ///*

	//  🟢 Ogre resources
	Ogre::SceneNode* node =0;
	Ogre::Item* it =0, *it2 =0;
	Ogre::String sMesh;
	// Ogre::MeshPtr mesh;  Ogre::v1::MeshPtr mesh1;

	void Create();
	void Destroy();
};


//  grid cells map
//---------------------------------------------------------
struct GridCells
{
	//std::vector<std::vector<std::vector<GridCellLods>>> g3;  //?
	// GridCellLods g[10][10][10];
	//std::vector<GridCellLods> g1;  //1d  y*x* + x* + *
	std::map<GridPos, GridCellLods> cells;
	
	GridPos From(Ogre::Vector3 worldPos);
	Ogre::Real fCellSize = 100.f;  // par
	
	void Clear();
	void AddMesh(int lod,
		Ogre::Vector3 worldPos,
		GridMtr mtr,
		const std::vector<Ogre::Vector3>& pos, const std::vector<Ogre::Vector3>& norm,
		const std::vector<Ogre::Vector4>& clr, const std::vector<Ogre::Vector2>& tcs,
		const std::vector<Ogre::uint16>& idx);

	void Create();
	void Destroy();
};


///  ***  GRID  🏗️ MESH   🏛️ Columns  WIP  ****
//   whole grid, all mtrs
//---------------------------------------------------------
struct GridMtrs
{
	// std::vector<GridCells> mtrs;
	// std::map<Ogre::String, int> mtrMap;
	std::map<GridMtr, GridCells> mtrs;
	
	void Clear();
	void AddMesh(int lod,
		Ogre::Vector3 worldPos,
		GridMtr mtr,
		const std::vector<Ogre::Vector3>& pos, const std::vector<Ogre::Vector3>& norm,
		const std::vector<Ogre::Vector4>& clr, const std::vector<Ogre::Vector2>& tcs,
		const std::vector<Ogre::uint16>& idx);
	
	void Create();
	void Destroy();
};
