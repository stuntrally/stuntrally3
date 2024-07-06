#include "pch.h"
#include <OgreHlms.h>
#include "Def_Str.h"
#include "RenderConst.h"
#include "SceneXml.h"
#include "CScene.h"
#include "Road.h"  // sun rot
#include "FluidsXml.h"
#include "dbl.h"
#ifdef SR_EDITOR
	#include "CApp.h"
	#include "settings.h"
#else
	#include "CGame.h"
	#include "settings.h"
	#include "CarModel.h"
#endif
#include <OgreCommon.h>
#include <OgreRoot.h>
#include <OgreException.h>
#include <OgreSceneNode.h>
#include <OgreCamera.h>
#include <OgreParticleSystem.h>
#include <OgreParticleEmitter.h>

#include <OgreItem.h>
#include <OgreMesh.h>
#include <OgreMeshManager.h>
#include <OgreMesh2.h>
#include <OgreMeshManager2.h>
#include <OgreManualObject2.h>

#include <OgreHlmsPbsDatablock.h>
#include <OgreHlmsPbsPrerequisites.h>

#include <OgreAtmosphereComponent.h>
#include "Atmosphere.h"
#include "OgreHlmsPbsTerraShadows.h"
using namespace Ogre;


//  🌟 all
void CScene::CreateAllAtmo()
{
	CreateSun();
	CreateFog();
	CreateWeather();
	CreateSkyDome(sc->skyMtr, sc->skyYaw);
}
void CScene::DestroyAllAtmo()
{
	DestroySkyDome();
	DestroyWeather();
	DestroyFog();
	DestroySun();
}


//  💡 Light
//-------------------------------------------------------------------------------------
void CScene::CreateSun()
{
	LogO("C--- create Sun");
	auto *mgr = app->mSceneMgr;
	SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );

	//  More Lights  //** higher CPU use, bad for debug
	auto& li = app->pSet->li;
	if (li.front || li.collect)  // sum all li ?-
	{
		switch (li.grid_quality)
		{
// bEnable, width, height, numSlices |  lightsPerCell, decalsPerCell, cubeProbesPerCel  | float minDist, maxDist
		case 0:  mgr->setForwardClustered(true, 16,  8, 24,   4, 0, 2,  2, 50);  break;
		case 1:  mgr->setForwardClustered(true, 24, 12, 24,   8, 0, 2,  2, 100);  break;
		case 2:  mgr->setForwardClustered(true, 32, 16, 24,   8, 0, 2,  2, 100);  break;
		case 3:  mgr->setForwardClustered(true, 32, 16, 24,  16, 0, 2,  2, 150);  break;
		// case 2:  mgr->setForwardClustered(true, 64, 32, 24,  32, 0, 2,  2, 150);  break;  // 30 fps
		// case 3:  mgr->setForwardClustered(true,128, 64,  8,  96, 0, 2,  5, 500);  break;  // 15 fps
	}	}
	else
        mgr->setForwardClustered( false, 0, 0, 0, 0, 0, 0, 0, 0 );

	sun = mgr->createLight();
	sun->setType( Light::LT_DIRECTIONAL );

	ndSun = rootNode->createChildSceneNode();
	ndSun->attachObject( sun );

	// sun->setPowerScale( 1.f );  // should be 1 with HDR
	sun->setPowerScale( Math::PI );  //** par
	
	UpdSun();
}

void CScene::DestroySun()
{
	auto *mgr = app->mSceneMgr;
	if (sun)
		mgr->destroyLight(sun);
	sun = 0;

	if (ndSun)
		mgr->destroySceneNode(ndSun);
	ndSun = 0;
}


//  🌫️ Fog / Atmosphere
//-------------------------------------------------------------------------------------
void CScene::CreateFog()
{
	LogO("C--- create Atmosphere");
	auto *mgr = app->mSceneMgr;

	atmo = OGRE_NEW Atmosphere( app->mRoot->getRenderSystem()->getVaoManager(), mgr );
	
	OGRE_ASSERT_HIGH( dynamic_cast<Atmosphere*>( mgr->getAtmosphere() ) );
	atmo = static_cast<Atmosphere*>( mgr->getAtmosphere() );

	UpdFog();
}	

void CScene::DestroyFog()
{
	LogO("D--- destroy Atmosphere");
	auto *mgr = app->mSceneMgr;

	AtmosphereComponent *atm = mgr->getAtmosphereRaw();
	OGRE_DELETE atm;
	atmo = 0;
}


//  🌫️ Fog set Atmo params
//-------------------------------------------------------------------------------------
void CScene::UpdFog(bool on, bool off)
{
	if (!atmo || !sun)  return;
	bool fog = off ? false : app->pSet->bFog || on;

	atmo->fogStartDistance = sc->fogStart;
	atmo->fogDensity = !fog ? 0.000001f :
		4000.f / sc->fogEnd * 0.0001f;  //par
		// 4000.f / (sc->fogEnd - sc->fogStart) * 0.0001f;
	atmo->fogHcolor = sc->fogClrH.GetRGBA();
	atmo->fogHparams = !fog ? Vector4(
		-10000.f,
		0.001f,
		0.000001f, 0)
	: Vector4(
		sc->fogHeight - sc->fogHDensity,
		1.f/sc->fogHDensity,
		2000.f / sc->fogHEnd * 0.0004f,  //par
		// 2000.f / (sc->fogHEnd - sc->fogHStart) * 0.0004f,
		sc->fogHStart);

	//sc->fogHeight, ok ? 1.f/sc->fogHDensity : 0.f,
	//sc->fogHStart, 1.f/(sc->fogHEnd - sc->fogHStart);
	atmo->fogColourSun = sc->fogClr.GetRGBA();
	atmo->fogColourAway = sc->fogClr2.GetRGBA();

 	// par  not on gui..
	atmo->fogBreakMinBrightness = 0.25f;
	atmo->fogBreakFalloff = 0.1f;
}


//  ⛅ Sky dome
//-------------------------------------------------------------------------------------
void CScene::UpdSkyScale()
{
	const float sc[SK_ALL] =
	{  app->pSet->g.view_distance, app->pSet->g.refl_dist, app->pSet->g.water_dist };

	for (int i=0; i < SK_ALL; ++i)
	if (ndSky[i])
	{
		Vector3 scale = sc[i] * Vector3::UNIT_SCALE * 0.7f;
		// LogO("Sky "+toStr(i)+" "+fToStr(sc[i]));
		ndSky[i]->setScale(scale);
		ndSky[i]->_getFullTransformUpdated();
	}
	// ndSky[0]->setVisible(0);
	// ndSky[1]->setVisible(1);
	// ndSky[2]->setVisible(0);
}

void CScene::CreateSkyDome(String sMater, float yaw)
{
	// return;  //** test no sky
	if (itSky[0])  return;  // already created
	LogO("C--- create SkyDomes");
	
	auto *mgr = app->mSceneMgr;
	ManualObject* m = mgr->createManualObject();
	m->begin(sMater, OT_TRIANGLE_LIST);

	//  divisions- quality
	int ia = 32*2, ib = 24,iB = 24 +1/*below_*/, i=0;
	
	//  angles, max
	const Real B = Math::HALF_PI, A = Math::TWO_PI;
	Real bb = B/ib, aa = A/ia;  // add
	Real a,b;
	ia += 1;

	//  up/dn y  )
	for (b = 0.f; b <= B+bb/*1*/*iB; b += bb)
	{
		Real cb = sinf(b), sb = cosf(b);
		Real y = sb;

		//  circle xz  o
		for (a = 0.f; a <= A; a += aa, ++i)
		{
			Real x = cosf(a)*cb, z = sinf(a)*cb;
			m->position(x,y,z);
			m->normal(-x,0,-z);

			m->textureCoord(a/A, b/B);

			if (a > 0.f && b > 0.f)  // rect 2tri
			{
				m->index(i-1);  m->index(i);     m->index(i-ia);
				m->index(i-1);  m->index(i-ia);  m->index(i-ia-1);
			}
	}	}
	m->end();


	//  todo: sky in postprocess shader? splits workspace etc
	//  ndSky is moved each frame to camera pos to be in center
	const uint32 vis[SK_ALL] = { RV_SkyMain, RV_SkyCubeRefl, RV_SkyPlanarRefl };
	auto dyn = SCENE_DYNAMIC; // SCENE_STATIC;
	for (int i=0; i < SK_ALL; ++i)
	{
		sMeshSky[i] = "skymesh"+toStr(i);
		MeshPtr mesh = m->convertToMesh(sMeshSky[i], "General", false);
		try
		{	itSky[i] = app->mSceneMgr->createItem( mesh, dyn );
			itSky[i]->setCastShadows(false);
			itSky[i]->setVisibilityFlags(vis[i]);
			// LogO(toStr(vis[i]));
			// Aabb aabb( Aabb::BOX_INFINITE );  itSky->setLocalAabb(aabb);  // meh-
			
			ndSky[i] = app->mSceneMgr->getRootSceneNode( dyn )->createChildSceneNode( dyn );
			ndSky[i]->attachObject(itSky[i]);
			Quaternion q;  q.FromAngleAxis(Degree(-yaw), Vector3::UNIT_Y);
			ndSky[i]->setOrientation(q);
		}
		catch (Exception e)
		{
			LogO("C--- create Sky failed!");
		}
	}
	UpdSkyScale();
	
	//  tex change to mirror
return;  //!
	Root *root = app->mRoot;
	HlmsSamplerblock sampler;
	sampler.mMinFilter = FO_ANISOTROPIC;  sampler.mMagFilter = FO_ANISOTROPIC;
	sampler.mMipFilter = FO_LINEAR; //?FO_ANISOTROPIC;
	sampler.mMaxAnisotropy = app->pSet->g.anisotropy;
	auto w = TAM_MIRROR;
	sampler.mU = w;  sampler.mV = w;  sampler.mW = w;

	Hlms *hlms = root->getHlmsManager()->getHlms( HLMS_PBS );
	HlmsPbsDatablock *datablock = static_cast<HlmsPbsDatablock*>(hlms->getDatablock(sMater));
	datablock->setSamplerblock( PBSM_EMISSIVE, sampler );
}

void CScene::DestroySkyDome()
{
	LogO("D--- destroy SkyDomes");
	auto *mgr = app->mSceneMgr;
	for (int i=0; i < SK_ALL; ++i)
	{
		if (!sMeshSky[i].empty())
		{	MeshManager::getSingleton().remove(sMeshSky[i]);  sMeshSky[i].clear();  }
		if (ndSky[i]){  mgr->destroySceneNode(ndSky[i]);  ndSky[i] = 0;  }
		if (itSky[i]){  mgr->destroyItem(itSky[i]);  itSky[i] = 0;  }
	}
}


//  🌞 Sun
//-------------------------------------------------------------------------------------
void CScene::UpdSky()
{
	Quaternion q;  q.FromAngleAxis(Degree(-sc->skyYaw), Vector3::UNIT_Y);
	for (int i=0; i < SK_ALL; ++i)
	if (ndSky[i])
	{
		ndSky[i]->setOrientation(q);
		ndSky[i]->_getFullTransformUpdated();
	}
	UpdSun();  //in ed
}

//  upd sun
void CScene::UpdSun(float dt)
{
	if (!sun)  return;
	Vector3 dir = SplineRoad::GetRot(sc->ldYaw - sc->skyYaw, -sc->ldPitch);
	// sun->setDiffuseColour( 0.0, 1.0, 0.0);
	// sun->setSpecularColour(0.0, 0.0, 1.0);

	sun->setDirection(dir);
	ndSun->_getFullTransformUpdated();

	sun->setPowerScale( Math::PI );  // 💡 brightness simple
	float bright = 0.9f*1.3f * app->pSet->bright, contrast = app->pSet->contrast;
	sun->setDiffuseColour( sc->lDiff.GetClr() * 2.2f  * bright * contrast);
	sun->setSpecularColour(sc->lSpec.GetClr() * 0.75f * bright * contrast);

	//  🌒 ambient
	app->mSceneMgr->setAmbientLight(
		sc->lAmb.GetClr() * 1.2f * bright / contrast,
		sc->lAmb.GetClr() * 1.2f * bright / contrast,
		// ColourValue( 1.f, 0.f, 0.f ) * 0.1f,
		// ColourValue( 0.f, 1.f, 0.f ) * 0.1f,
		-dir);
		// 0.8f, 0x0 );  // env? EnvFeatures_DiffuseGiFromReflectionProbe
		// -dir + Ogre::Vector3::UNIT_Y * 0.2f );

	//  🌪️ inc time for  water, grass wind, etc
	if (atmo && dt > 0.f)
		atmo->globalTime.x += dt;
	

	//  🌊 underwater
	///~~  fluid fog, send params to shaders
#ifndef SR_EDITOR  // game only
	bool hasCars = !app->carModels.empty();
	if (atmo &&  hasCars  && app->pSet->game.local_players == 1)  // todo: splitscreen
	{
		int fi = app->carModels[0]->iCamFluid;
		float p = app->carModels[0]->fCamFl;
		if (fi >= 0)
		{	const FluidBox* fb = &app->scn->sc->fluids[fi];
			const FluidParams& fp = app->scn->sc->pFluidsXml->fls[fb->id];

			// LogO(fToStr(fb->pos.y)+" "+fToStr(p)+"  clr "+
			// fToStr(fp.fog.r)+" "+fToStr(fp.fog.g)+" "+fToStr(fp.fog.b)+" "+fToStr(fp.fog.a));
			atmo->fogFluidH = Vector4(
				fb->pos.y +p /*+0.5f par? ofsH..*/, 1.f / fp.fog.dens, fp.fog.densH +p*0.5f, 0);

			atmo->fogFluidClr = Vector4(
				fp.fog.r, fp.fog.g, fp.fog.b, fp.fog.a);
		}else
		{	atmo->fogFluidH = Vector4(
				-1900.f, 1.f/17.f, 0.15f, 0);
			atmo->fogFluidClr = Vector4(
				0,0,0,0);
		}
	}

	//  🌿 grass sphere pos ()
	if (atmo)
	if (hasCars)
	{
		Real r = 1.7;  r *= r;  //par
		auto& p = app->carModels[0]->posSph[0];
		atmo->posSph0 = Vector4(p.x,p.y,p.z,r);
		p = app->carModels[0]->posSph[1];
		atmo->posSph1 = Vector4(p.x,p.y,p.z,r);
	}else
	{	atmo->posSph0 = Vector4(0,0,500,-1);
		atmo->posSph1 = Vector4(0,0,500,-1);
	}

#endif	

	//  wind

	//  post, gamma-
}


//  🌧️ Weather  rain,snow
//-------------------------------------------------------------------------------------
void CScene::CreateWeather()
{
	LogO("C--- create Weather");
	if (!pr && !sc->rainName.empty())
	{	try
		{	pr = app->mSceneMgr->createParticleSystem(sc->rainName);
		}catch (Exception& ex)
		{	LogO("Warning: particle_system: " + sc->rainName + " doesn't exist");
			return;
		}
		pr->setVisibilityFlags(RV_Particles);
		app->mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pr);
		// pr->setRenderQueueGroup(RQG_Weather);
		pr->getEmitter(0)->setEmissionRate(0);
		pr->_update(2.f);  // emit, started 2 sec ago
	}
	if (!pr2 && !sc->rain2Name.empty())
	{	try
		{	pr2 = app->mSceneMgr->createParticleSystem(sc->rain2Name);
		}catch (Exception& ex)
		{	LogO("Warning: particle_system: " + sc->rainName + " doesn't exist");
			return;
		}
		pr2->setVisibilityFlags(RV_Particles);
		app->mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pr2);
		// pr2->setRenderQueueGroup(RQG_Weather);
		pr2->getEmitter(0)->setEmissionRate(0);
		pr2->_update(2.f);
	}
}
void CScene::DestroyWeather()
{
	LogO("D--- destroy Weather");
	if (pr)  {  app->mSceneMgr->destroyParticleSystem(pr);   pr=0;  }
	if (pr2) {  app->mSceneMgr->destroyParticleSystem(pr2);  pr2=0;  }
}

//  💫🌧️ Update Weather
void CScene::UpdateWeather(Camera* cam, float invDT, float emitMul)
{
	const Vector3& pos = cam->getPosition(), dir = cam->getDirection();
	static Vector3 oldPos = pos, oldDir = dir;

	Vector3 vel = (pos-oldPos) * invDT;
	Vector3 rot = (dir-oldDir) * invDT;

	//  try to predict where to emit
	//  given movement and rotation speeds
	Vector3 par = pos
		+ (dir + 0.6f * rot) * 12.f  // in front
		// + rot * 1.f  //par rot effect on emitter pos
		+ vel * 0.2f;  //par move effect on emitter pos
	
	oldPos = pos;  oldDir = dir;

	if (pr && sc->rainEmit > 0)
	{
		auto* pe = pr->getEmitter(0);
		pe->setPosition(par);
		pe->setEmissionRate(emitMul * sc->rainEmit);
	}
	if (pr2 && sc->rain2Emit > 0)
	{
		auto* pe = pr2->getEmitter(0);
		pe->setPosition(par);
		pe->setEmissionRate(emitMul * sc->rain2Emit);
	}
}
