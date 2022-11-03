#include "pch.h"
#include <OgreHlms.h>
#include "Def_Str.h"
#include "RenderConst.h"
#include "data/SceneXml.h"
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

#include <OgreHlmsUnlitDatablock.h>
#include <OgreHlmsPbsPrerequisites.h>

// #include "TerrainGame.h"
#include "OgreAtmosphereComponent.h"
#include "OgreAtmosphereNpr.h"
using namespace Ogre;




//  Light
//-------------------------------------------------------------------------------------{
void CScene::CreateSun()
{
	SceneManager *mgr = app->mSceneMgr;
	mgr->destroyAllLights();
	LogO("---- new sun light");
	sun = mgr->createLight();
	sun->setType( Light::LT_DIRECTIONAL );

	SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );
	SceneNode *lightNode = rootNode->createChildSceneNode();
	lightNode->attachObject( sun );

	// mSunLight->setPowerScale( 1.f );  // should be * 1..
	sun->setPowerScale( Math::PI * 2.5 );  //** par! 1.5 2 3* 4
	UpdSun();


	//  Atmosphere  ------------------------------------------------
	LogO("---- new Atmosphere");
	AtmosphereComponent *atmosphere = mgr->getAtmosphereRaw();
	OGRE_DELETE atmosphere;

	atmo = OGRE_NEW AtmosphereNpr( app->mRoot->getRenderSystem()->getVaoManager() );
	{
		// Preserve the Power Scale explicitly set by the sample
		AtmosphereNpr::Preset preset = atmo->getPreset();
		preset.linkedLightPower = sun->getPowerScale();
		atmo->setPreset( preset );
	}

	atmo->setSunDir( sun->getDirection(), sc->ldPitch / 180.f );
		// std::asin( Math::Clamp( -sun->getDirection().y, -1.0f, 1.0f ) ) / Math::PI );
	atmo->setLight( sun );
	atmo->setSky( mgr, true );
	
	OGRE_ASSERT_HIGH( dynamic_cast<AtmosphereNpr*>( mgr->getAtmosphere() ) );
	atmo = static_cast<AtmosphereNpr*>( mgr->getAtmosphere() );
	

	AtmosphereNpr::Preset p = atmo->getPreset();
	p.fogDensity = 2000.f / sc->fogEnd * 0.001f; //0.0002f;  //** par
	p.densityCoeff = 0.27f;  //0.47f;
	p.densityDiffusion = 0.75f;  //2.0f;
	p.horizonLimit = 0.025f;
	// p.sunPower = 1.0f;
	// p.skyPower = 1.0f;
	p.skyColour = sc->fogClr.GetRGB1(); //Vector3(0.234f, 0.57f, 1.0f);
	p.fogBreakMinBrightness = 0.25f;
	p.fogBreakFalloff = 0.1f;
	// p.linkedLightPower = Math::PI;
	// p.linkedSceneAmbientUpperPower = 0.1f * Math::PI;
	// p.linkedSceneAmbientLowerPower = 0.01f * Math::PI;
	p.envmapScale = 1.0f;
	atmo->setPreset( p );
}


//  Sky dome
//-------------------------------------------------------------------------------------
// void CScene::CreateSkyDome(String sMater, float sc, float yaw)
void CScene::CreateSkyDome(String sMater, float yaw)
{
	if (moSky)  return;
	Vector3 scale = 15000 * Vector3::UNIT_SCALE;
	// Vector3 scale = pSet->view_distance * Vector3::UNIT_SCALE * 0.7f;
	
	SceneManager *mgr = app->mSceneMgr;
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

			m->textureCoord(a/A, b/B);

			if (a > 0.f && b > 0.f)  // rect 2tri
			{
				m->index(i-1);  m->index(i);     m->index(i-ia);
				m->index(i-1);  m->index(i-ia);  m->index(i-ia-1);
			}
		}
	}
	m->end();
	moSky = m;

	Aabb aab(Vector3(0,0,0), Vector3(1,1,1)*1000000);
	m->setLocalAabb(aab);  // always visible
	//m->setRenderQueueGroup(230);  //?
	m->setCastShadows(false);

	ndSky = mgr->getRootSceneNode()->createChildSceneNode();
	ndSky->attachObject(m);
	ndSky->setScale(scale);
	Quaternion q;  q.FromAngleAxis(Degree(-yaw), Vector3::UNIT_Y);
	ndSky->setOrientation(q);

	//  change to wrap..
	Root *root = app->mRoot;
	Hlms *hlms = root->getHlmsManager()->getHlms( HLMS_UNLIT );
	HlmsUnlitDatablock *datablock = static_cast<HlmsUnlitDatablock*>(hlms->getDatablock(sMater));
	HlmsSamplerblock sampler( *datablock->getSamplerblock( PBSM_DIFFUSE ) );  // copy
	sampler.mU = TAM_MIRROR;  sampler.mV = TAM_MIRROR;  sampler.mW = TAM_MIRROR;
	datablock->setSamplerblock( PBSM_DIFFUSE, sampler );
}

void CScene::DestroySkyDome()
{
	LogO("---- destroy SkyDome");
	SceneManager *mgr = app->mSceneMgr;
	if (moSky)
	{   mgr->destroyManualObject(moSky);  moSky = 0;  }
	if (ndSky)
	{   mgr->destroySceneNode(ndSky);  ndSky = 0;  }
}


//  Sun
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
	sun->setDirection(dir);
	sun->setDiffuseColour( sc->lDiff.GetClr());
	sun->setSpecularColour(sc->lSpec.GetClr());
	//; app->mSceneMgr->setAmbientLight(sc->lAmb.GetClr());

	//  ambient
	app->mSceneMgr->setAmbientLight(
		// ColourValue( 0.63f, 0.61f, 0.28f ) * 0.04f,
		// ColourValue( 0.52f, 0.63f, 0.76f ) * 0.04f,
		// ColourValue( 0.33f, 0.61f, 0.98f ) * 0.01f,
		// ColourValue( 0.02f, 0.53f, 0.96f ) * 0.01f,

		// ColourValue( 0.93f, 0.91f, 0.38f ) * 0.04f,
		// ColourValue( 0.22f, 0.53f, 0.96f ) * 0.04f,
		// ColourValue( 0.33f, 0.61f, 0.98f ) * 0.01f,
		// ColourValue( 0.02f, 0.53f, 0.96f ) * 0.01f,
		ColourValue( 0.99f, 0.94f, 0.90f ) * 0.04f,  //** par
		ColourValue( 0.90f, 0.93f, 0.96f ) * 0.04f,
		// ColourValue( 0.3f, 0.5f, 0.7f ) * 0.1f * 0.75f,
		// ColourValue( 0.6f, 0.45f, 0.3f ) * 0.065f * 0.75f,
		-dir );
		// -dir + Ogre::Vector3::UNIT_Y * 0.2f );

	if (atmo)
		atmo->setSunDir( sun->getDirection(), sc->ldPitch / 180.f );
}

//  Fog
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


//  Weather  rain,snow
//-------------------------------------------------------------------------------------
void CScene::CreateWeather()
{
	if (!pr && !sc->rainName.empty())
	{	try
		{	pr = app->mSceneMgr->createParticleSystem(sc->rainName);
		}catch (Exception& ex)
		{	LogO("Warning: particle_system: " + sc->rainName + " doesn't exist");
			return;
		}
		pr->setVisibilityFlags(RV_Particles);
		app->mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pr);
		pr->setRenderQueueGroup(RQG_Weather);
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
		pr2->setRenderQueueGroup(RQG_Weather);
		pr2->getEmitter(0)->setEmissionRate(0);
		pr2->_update(2.f);
	}
}
void CScene::DestroyWeather()
{
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
