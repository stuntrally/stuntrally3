#include "pch.h"
#include "PreviewScene.h"
#include "RenderConst.h"
#include "Def_Str.h"
#include "paths.h"
#include "App.h"
#include "SColor.h"
#include "Road.h"

#include "GraphicsSystem.h"
#include "settings.h"
#include <OgreCommon.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreException.h>
#include <OgreResourceGroupManager.h>
#include <OgrePlatformInformation.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreItem.h>
#include <OgreSubMesh2.h>
using namespace Ogre;


//  🆕 Create
bool PreviewScene::Create(App* app1)
{
	app = app1;
	LogO("C--P PreviewScene Create");

	const size_t numThreads = std::max<size_t>( 1, PlatformInformation::getNumLogicalCores() );

	mgr = app->mGraphicsSystem->mRoot->createSceneManager(
		ST_GENERIC, numThreads, "PreviewSM" );

	//  🌞 Sun
	sun = mgr->createLight();
	sun->setType( Light::LT_DIRECTIONAL );

	ndSun = mgr->getRootSceneNode( SCENE_STATIC )->createChildSceneNode();
	ndSun->attachObject( sun );

	sun->setPowerScale( Math::PI );  //** par

	float ldPitch = 55.f, ldYaw = 133.f;  // dir angles
	SColor
		lAmb (0.4f, 0.f, 0.8f),  // par
		lDiff(0.4f, 0.f, 0.6f),
		lSpec(0.4f, 0.f, 0.5f);

	Vector3 dir = SplineRoad::GetRot(ldYaw, -ldPitch);
	sun->setDirection(dir);
	ndSun->_getFullTransformUpdated();

	float bright = 0.9f*1.3f * 1.f, contrast = 1.f;
	sun->setDiffuseColour( lDiff.GetClr() * 2.2f  * bright * contrast);
	sun->setSpecularColour(lSpec.GetClr() * 0.75f * bright * contrast);

	//  🌒 ambient
	mgr->setAmbientLight(
		lAmb.GetClr() * 1.2f * bright / contrast,
		lAmb.GetClr() * 1.2f * bright / contrast,
		-dir);

	//  camera
	cam = mgr->createCamera( "PreviewCam3D", true );
	cam->setPosition(Vector3(0, 20, 20));
	cam->lookAt(Vector3(0,2,0));
	cam->setNearClipDistance(0.5);
	cam->setFarClipDistance(10000);
	cam->setAspectRatio(1.0);
	// ndCam?	
	// ws = 0;

	// mgr->_setAtmosphere(app->atmo)

	return true;
}

//  💥 Destroy
void PreviewScene::Destroy()
{
	mgr->_setAtmosphere(0);

	LogO("D--P PreviewScene Destroy");
	Unload();

	if (cam)    mgr->destroyCamera(cam);  cam = 0;
	if (sun)    mgr->destroyLight(sun);  sun = 0;
	if (ndSun)  mgr->destroySceneNode(ndSun);  ndSun = 0;

	// Ogre::CompositorWorkspace* ws = 0;
	if (mgr)  app->mGraphicsSystem->mRoot->destroySceneManager(mgr);
}

void PreviewScene::Unload()
{
	LogO("=--P PreviewScene Unload");
	if (node)  mgr->destroySceneNode(node);  node = 0;
	if (item)  mgr->destroyItem(item);  item = 0;
}


//  🔁 Load mesh
bool PreviewScene::Load(Ogre::String mesh)
{
	if (!mgr)  return false;
	if (node)
		Unload();
	if (mesh.empty())  return false;
	
	LogO("+--P PreviewScene Load: "+mesh);

	try
	{	item = mgr->createItem(mesh);
		
		//  txt for info  --------
		sInfo.clear();
		int all = item->getNumSubItems(), all_tris = 0, lods = 0;
		for (int i=0; i < all; ++i)
		{
			auto* si = item->getSubItem(i);
			if (!i)  lods = si->getSubMesh()->mVao[0].size();  // once
			int tris = si->getSubMesh()->mVao[0][0]->getPrimitiveCount();
			sInfo += " "+si->getSubMesh()->getMaterialName()+"    "+fToStr(tris /1000.f,1,4)+" k  "+"\n";
			all_tris += tris;
		}
		sTotal = ": "+toStr(all)+"  Tris: "+fToStr(all_tris /1000.f,0,2)+" k" + "  LODs: "+toStr(lods);
		//  lods  --------
		sLods.clear();
		for (int lod=0; lod < lods; ++lod)
		{
			int lod_tris = 0;
			for (int i=0; i < all; ++i)
			{
				auto* si = item->getSubItem(i);
				int lods = si->getSubMesh()->mVao[0].size();
				if (lod < lods)
				{	int tris = si->getSubMesh()->mVao[0][lod]->getPrimitiveCount();
					lod_tris += tris;
			}	}
			sLods += " "+toStr(lod+1)+"    "+fToStr(lod_tris /1000.f,1,4)+" k   "+
				fToStr(100.f * lod_tris / all_tris,1,4)+"%\n";
		}
		
		node = mgr->getRootSceneNode()->createChildSceneNode();
		node->attachObject(item);
	}
	catch(Exception e)
	{
		LogO("+--P PreviewScene can't create!");
		return false;
	}

	//  🎥 set camera to fit
	Aabb ab = item->getLocalAabb();
	dim = ab.getSize();
	
	AxisAlignedBox box;
	box.merge(ab.getMinimum());
	box.merge(ab.getMaximum());
	if (box.isNull())  return false;

	Vector3 vec = box.getSize();
	float width = sqrt(vec.x * vec.x + vec.z * vec.z), height = vec.y;
	float len2 = width / cam->getAspectRatio(), len1 = height;
	if (len1 < len2)  len1 = len2;
	len1 /= 0.86;  // [sqrt(3)/2] for 60 degrees field of view

	Vector3 pos = box.getCenter();
	pos.z += vec.z / 2 + len1 + 1/* min dist*/;
	pos += Vector3(0, height * 0.9f/* pitch*/, len1 * 0.1f);
	pos *= 0.85f;  //* closer
	Vector3 look = Vector3(0, box.getCenter().y * 0.8f, 0);

	cam->setPosition(pos);
	cam->lookAt(look);
	// cam->_getFullTransformUpdated();

	yaw = 0.f;
	return true;
}

void PreviewScene::Update(float dt)
{
	if (!node)  return;

	Quaternion q;  q.FromAngleAxis(Radian(yaw), Vector3::UNIT_Y);
	node->setOrientation(q);
	node->_getFullTransformUpdated();
#ifdef SR_EDITOR
	yaw += dt * 0.7f * app->pSet->prv_rot_speed;  //par rot speed
#endif
}
