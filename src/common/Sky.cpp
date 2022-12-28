#include "pch.h"
#include <OgreHlms.h>
#include "Def_Str.h"
#include "RenderConst.h"
#include "SceneXml.h"
#include "CScene.h"
#include "Road.h"  // sun rot
#include "dbl.h"
#ifdef SR_EDITOR
	#include "CApp.h"
	#include "settings.h"
#else
	#include "CGame.h"
	#include "settings.h"
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
using namespace Ogre;


//  🌟 all
void CScene::CreateAllAtmo()
{
	CreateSun();
	CreateFog();
	CreateWeather();
	CreateSkyDome(sc->skyMtr, /*app->pSet->view_distance,*/ sc->skyYaw);
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
	LogO("C--- create sun");
	auto *mgr = app->mSceneMgr;
	SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );
	
	mgr->setForwardClustered( true, 16, 8, 24, 4, 0, 2, 2, 50 );  // for more lights

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
	atmo->setLight( sun );
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
void CScene::UpdFog()
{
	if (!atmo || !sun)  return;
	bool no = !app->pSet->bFog;
	Atmosphere2Npr::Preset p = atmo->getPreset();

	p.fogDensity = no ? 0.000001f :
		4000.f / sc->fogEnd * 0.0001f;  //** par
	p.fogHcolor = sc->fogClrH.GetRGBA();
	p.fogHparams = no ?	Vector4(
		-10000.f,
		0.001f,
		0.000001f, 0)
	: Vector4(
		sc->fogHeight - sc->fogHDensity,
		1.f/sc->fogHDensity,
		2000.f / sc->fogHEnd * 0.0004f, 0);

	p.densityCoeff = 0.27f;  //0.47f;
	p.densityDiffusion = 0.75f;  //2.0f;
	p.horizonLimit = 0.025f;
	p.sunPower = 1.0f;  // par gui..
	p.skyPower = 1.0f;
	// p.skyColour = Vector3(0,0.5,1);// sc->fogClr2.GetRGB();
	p.skyColour = sc->fogClr2.GetRGB1(); //Vector3(0.234f, 0.57f, 1.0f);
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
}
void CScene::CreateSkyDome(String sMater, float yaw)
{
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
	UpdSkyScale();
	Quaternion q;  q.FromAngleAxis(Degree(-yaw), Vector3::UNIT_Y);
	ndSky->setOrientation(q);


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
	Quaternion q;  q.FromAngleAxis(Degree(-sc->skyYaw), Vector3::UNIT_Y);
	ndSky->setOrientation(q);
	UpdSun();  //in ed
}

//  upd sun
void CScene::UpdSun()
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

	sun->setPowerScale( Math::PI );
	sun->setDiffuseColour( sc->lDiff.GetClr() * 2.2f );
	sun->setSpecularColour(sc->lSpec.GetClr() * 0.75f );

	//  ambient
	app->mSceneMgr->setAmbientLight(
		sc->lAmb.GetClr() * 1.2f,
		sc->lAmb.GetClr() * 1.2f,
		// ColourValue( 1.f, 0.f, 0.f ) * 0.1f,
		// ColourValue( 0.f, 1.f, 0.f ) * 0.1f,
		-dir);
		// 0.8f, 0x0 );  // env? EnvFeatures_DiffuseGiFromReflectionProbe
		// -dir + Ogre::Vector3::UNIT_Y * 0.2f );

}

#if 0
//  Fog old?-
void CScene::UpdFog(bool bForce)
{
	const ColourValue clr(0.5,0.6,0.7,1);
	bool ok = !app->pSet->bFog || bForce;
	if (ok)
		app->mSceneMgr->setFog(FOG_LINEAR, clr, 1.f, sc->fogStart, sc->fogEnd);
	else
		app->mSceneMgr->setFog(FOG_NONE, clr, 1.f, 9000, 9200);

	/*Vector4 v;  // todo: fog
	v = sc->fogClr2.GetRGBA();
	v = sc->fogClr.GetRGBA();
	v = sc->fogClrH.GetRGBA();
	sc->fogHeight, ok ? 1.f/sc->fogHDensity : 0.f, sc->fogHStart, 1.f/(sc->fogHEnd - sc->fogHStart);*/
}
#endif


//  🌧️ Weather  rain,snow
//-------------------------------------------------------------------------------------
void CScene::CreateWeather()
{
	LogO("C--- create weather");
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
	LogO("D--- destroy weather");
	if (pr)  {  app->mSceneMgr->destroyParticleSystem(pr);   pr=0;  }
	if (pr2) {  app->mSceneMgr->destroyParticleSystem(pr2);  pr2=0;  }
}

void CScene::UpdateWeather(Camera* cam, float lastFPS, float emitMul)
{
	const Vector3& pos = cam->getPosition(), dir = cam->getDirection();
	static Vector3 oldPos = Vector3::ZERO;

	Vector3 vel = (pos-oldPos) * lastFPS;  oldPos = pos;
	Vector3 par = pos + dir * 12.f + vel * 0.6f;  //par move effect on emitter pos

	if (pr && sc->rainEmit > 0)
	{
		ParticleEmitter* pe = pr->getEmitter(0);
		pe->setPosition(par);
		pe->setEmissionRate(emitMul * sc->rainEmit);
	}
	if (pr2 && sc->rain2Emit > 0)
	{
		ParticleEmitter* pe = pr2->getEmitter(0);
		pe->setPosition(par);
		pe->setEmissionRate(emitMul * sc->rain2Emit);
	}
}

/*
void CScene::UpdPaceParams()  // pacenotes
{
	app->mFactory->setSharedParameter("paceParams", sh::makeProperty<sh::Vector4>(new sh::Vector4(
		app->pSet->pace_size, 5.f / app->pSet->pace_near,
		0.03f * app->pSet->pace_near, app->pSet->pace_alpha)));
}
*/
