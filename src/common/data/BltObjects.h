#pragma once
#include <vector>
#include <string>
#include <map>

// #include <Ogre.h>
#include <OgreVector3.h>


enum eBLTshape {  BLT_None=0, BLT_Sphere, BLT_CapsZ, BLT_Mesh, BLT_ALL };
const static std::string sBLTshape[BLT_ALL] = {"", "sphere", "capsZ", "mesh" };


struct BltShape
{
	eBLTshape type = BLT_Sphere;
	float radius = 1.f, height = 1.f;  // dims
	Ogre::Vector3 offset{0,0,0};  //Quaternion rot;  // pos, rot-
	
	float friction = 0.2f, restitution = 0.9f;  // collision pars
	float damping = 0.f;
};


class BltCollision
{
public:
	std::string mesh;  // for vegetation eg. tree.mesh
	Ogre::Vector3 offset{0,0,0};  // real mesh center, for placing on terrain
	std::vector <BltShape> shapes;
};


class BltObjects
{
public:
	std::map <std::string, bool> colNone;
	std::map <std::string, BltCollision> colsMap;
	std::map <std::string, BltCollision>::const_iterator colsMapFind;

	BltShape defPars;  // default if not found, for trimesh frict,rest params..

	bool LoadXml();
	const BltCollision* Find(std::string mesh);
};
