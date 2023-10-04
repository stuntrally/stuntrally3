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
#include <OgreAtmosphere2Npr.h>
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

	if (app->pSet->car_lights)  // for more lights  //** higher CPU use, bad for debug
		mgr->setForwardClustered( true, 16, 8, 24, 4, 0, 2, 2, 50 );
	else
		mgr->setForwardClustered( false, 16, 8, 24, 4, 0, 2, 2, 50 );

	sun = mgr->createLight();
	sun->setType( Light::LT_DIRECTIONAL );

	ndSun = rootNode->createChildSceneNode();
	ndSun->attachObject( sun );

	// sun->setPowerScale( 1.f );  // should be 1 with HDR
	sun->setPowerScale( Math::PI );  //** par! no HDR, * 2.5?  1.5 2 3* 4
	
	UpdSun();
}

void CScene::DestroySun()
{
	if (atmo)
		atmo->setLight(0);
	
	auto *mgr = app->mSceneMgr;
	if (sun)
		mgr->destroyLight(sun);
	sun = 0;

	if (ndSun)
		mgr->destroySceneNode( ndSun );
	ndSun = 0;
}


//  🌫️ Fog / Atmosphere
//-------------------------------------------------------------------------------------
void CScene::CreateFog()
{
	LogO("C--- create Atmosphere");
	auto *mgr = app->mSceneMgr;

	atmo = OGRE_NEW Atmosphere2Npr( app->mRoot->getRenderSystem()->getVaoManager() );

	atmo->setSunDir( sun->getDirection(), sc->ldPitch / 180.f );
	atmo->setLight( sun );  //-
	atmo->setSky( mgr, true );
	
	OGRE_ASSERT_HIGH( dynamic_cast<Atmosphere2Npr*>( mgr->getAtmosphere() ) );
	atmo = static_cast<Atmosphere2Npr*>( mgr->getAtmosphere() );

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

//  🌫️ Fog set Atmo params  ------
void CScene::UpdFog(bool on, bool off)
{
	if (!atmo || !sun)  return;
	bool fog = off ? false : app->pSet->bFog || on;
	Atmosphere2Npr::Preset p = atmo->getPreset();

	p.fogStartDistance = sc->fogStart;
	p.fogDensity = !fog ? 0.000001f :
		4000.f / sc->fogEnd * 0.0001f;  //** par`
		// 4000.f / (sc->fogEnd - sc->fogStart) * 0.0001f;  //** par-
	p.fogHcolor = sc->fogClrH.GetRGBA();
	p.fogHparams = !fog ? Vector4(
		-10000.f,
		0.001f,
		0.000001f, 0)
	: Vector4(
		sc->fogHeight - sc->fogHDensity,
		1.f/sc->fogHDensity,
		2000.f / sc->fogHEnd * 0.0004f,  //** par`
		// 2000.f / (sc->fogHEnd - sc->fogHStart) * 0.0004f,  //** par`
		sc->fogHStart);

	//sc->fogHeight, ok ? 1.f/sc->fogHDensity : 0.f,
	//sc->fogHStart, 1.f/(sc->fogHEnd - sc->fogHStart);
	p.fogColourSun = sc->fogClr.GetRGBA();
	p.fogColourAway = sc->fogClr2.GetRGBA();

 	// pars  not used, not on gui..
	p.densityCoeff = 0.27f;  //0.47f;
	p.densityDiffusion = 0.75f;  //2.0f;
	p.horizonLimit = 0.025f;
	p.sunPower = 1.0f;
	p.skyPower = 1.0f;
	// p.skyColour = Vector3(0,0.5,1);// sc->fogClr2.GetRGB();
	p.skyColour = sc->fogClr2.GetRGB1(); //?- Vector3(0.234f, 0.57f, 1.0f);
	
	p.fogBreakMinBrightness = 0.25f;
	p.fogBreakFalloff = 0.1f;
	p.linkedLightPower = sun->getPowerScale();
	p.linkedSceneAmbientUpperPower = 0.2f * Math::PI;  // ?
	p.linkedSceneAmbientLowerPower = 0.2f * Math::PI;
	p.envmapScale = 1.0f;
	atmo->setPreset( p );
}


//  ⛅ Sky dome
//-------------------------------------------------------------------------------------
void CScene::UpdSkyScale()
{
	if (!ndSky)  return;
	Vector3 scale = app->pSet->view_distance * Vector3::UNIT_SCALE * 0.7f;
	ndSky->setScale(scale);
	ndSky->_getFullTransformUpdated();
}
void CScene::CreateSkyDome(String sMater, float yaw)
{
	// return;  //** test no sky
	if (itSky)  return;
	LogO("C--- create SkyDome");
	
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

	sMeshSky = "skymesh";
	MeshPtr mesh = m->convertToMesh(sMeshSky, "General", false);
	auto dyn = SCENE_DYNAMIC; // SCENE_STATIC;  // todo: sky in postprocess shader..
	itSky = app->mSceneMgr->createItem( mesh, dyn );
	itSky->setCastShadows(false);
	itSky->setVisibilityFlags(RV_Sky);
	// Aabb aabb( Aabb::BOX_INFINITE );  itSky->setLocalAabb(aabb);  // meh-
	ndSky = app->mSceneMgr->getRootSceneNode( dyn )->createChildSceneNode( dyn );
	ndSky->attachObject(itSky);
	Quaternion q;  q.FromAngleAxis(Degree(-yaw), Vector3::UNIT_Y);
	ndSky->setOrientation(q);
	UpdSkyScale();


	//  tex change to mirror
	Root *root = app->mRoot;
	HlmsSamplerblock sampler;
	sampler.mMinFilter = FO_ANISOTROPIC;  sampler.mMagFilter = FO_ANISOTROPIC;
	sampler.mMipFilter = FO_LINEAR; //?FO_ANISOTROPIC;
	sampler.mMaxAnisotropy = app->pSet->anisotropy;
	auto w = TAM_MIRROR;
	sampler.mU = w;  sampler.mV = w;  sampler.mW = w;

	Hlms *hlms = root->getHlmsManager()->getHlms( HLMS_PBS );
	HlmsPbsDatablock *datablock = static_cast<HlmsPbsDatablock*>(hlms->getDatablock(sMater));
	datablock->setSamplerblock( PBSM_EMISSIVE, sampler );
}

void CScene::DestroySkyDome()
{
	LogO("D--- destroy SkyDome");
	auto *mgr = app->mSceneMgr;
	MeshManager::getSingleton().remove(sMeshSky);
	if (ndSky)
	{   mgr->destroySceneNode(ndSky);  ndSky = 0;  }
	if (itSky)
	{   mgr->destroyItem(itSky);  itSky = 0;  }
}


//  🌞 Sun
//-------------------------------------------------------------------------------------
void CScene::UpdSky()
{
	if (!ndSky)  return;
	Quaternion q;  q.FromAngleAxis(Degree(-sc->skyYaw), Vector3::UNIT_Y);
	ndSky->setOrientation(q);
	ndSky->_getFullTransformUpdated();
	UpdSun();  //in ed
}

//  upd sun
void CScene::UpdSun(float dt)
{
	if (!sun)  return;
	Vector3 dir = SplineRoad::GetRot(sc->ldYaw - sc->skyYaw, -sc->ldPitch);
	// sun->setDiffuseColour( 0.0, 1.0, 0.0);
	// sun->setSpecularColour(0.0, 0.0, 1.0);
	if (atmo)
	{	atmo->setSunDir( sun->getDirection(), sc->ldPitch / 180.f );
		atmo->setLight(sun);
	}
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
		atmo->globalTime += dt;
	

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

			LogO(fToStr(fb->pos.y)+" "+fToStr(p)+"  clr "+
			fToStr(fp.fog.r)+" "+fToStr(fp.fog.g)+" "+fToStr(fp.fog.b)+" "+fToStr(fp.fog.a));
			atmo->fogFluidH = Vector4(
				fb->pos.y +p /*+0.5f par? ofsH..*/, 1.f / fp.fog.dens, fp.fog.densH +p*0.5f, 0);

			atmo->fogFluidClr = Vector4(
				fp.fog.r, fp.fog.g, fp.fog.b, fp.fog.a);
		}else
		{	atmo->fogFluidH = Vector4(
				1900.f, 1.f/17.f, 0.15f, 0);
			atmo->fogFluidClr = Vector4(
				0,0,0,0);
		}
	}

	//  🌿 grass sphere pos ()
	/*if (hasCars)
	{
		Real r = 1.7;  r *= r;  //par
		const Vector3* p = &carModels[0]->posSph[0];
		mFactory->setSharedParameter("posSph0", sh::makeProperty <sh::Vector4>(new sh::Vector4(p->x,p->y,p->z,r)));
		p = &carModels[0]->posSph[1];
		mFactory->setSharedParameter("posSph1", sh::makeProperty <sh::Vector4>(new sh::Vector4(p->x,p->y,p->z,r)));
	}else
	{	mFactory->setSharedParameter("posSph0", sh::makeProperty <sh::Vector4>(new sh::Vector4(0,0,500,-1)));
		mFactory->setSharedParameter("posSph1", sh::makeProperty <sh::Vector4>(new sh::Vector4(0,0,500,-1)));
	}*/

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
