#include "pch.h"
#include "Def_Str.h"
#include "CData.h"
#include "ShapeData.h"
#include "GuiCom.h"
#include "CScene.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "Road.h"
#include "PaceNotes.h"
#include "Grass.h"
#include "pathmanager.h"
#include "RenderConst.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "enums.h"
#include "HudRenderable.h"
#include "GraphicsSystem.h"
#include <OgreTimer.h>
#include "Terra.h"
// #include <OgreTerrainGroup.h>
#include <OgreCommon.h>
#include <OgreHlmsCommon.h>
#include <OgreWindow.h>
#include <OgreOverlay.h>
#include <OgreOverlayElement.h>
#include <OgreMeshManager.h>
#include <OgreParticleSystem.h>
#include <OgreParticleEmitter.h>
#include <OgreManualObject.h>
#include <OgreViewport.h>
#include <OgreMaterialManager.h>
// #include <OgreTextureManager.h>
#include <OgreResourceGroupManager.h>
#include <OgreSceneNode.h>
#include "MessageBox.h"
#include <cmath>
// #include "Instancing.h"
using namespace Ogre;
using namespace std;


//  Create Scene
//-------------------------------------------------------------------------------------
void App::createScene01()  // once, init
{
	LogO(">>>>>>>> Init SR ed --------");
	
	//  SR cfg, xmls etc
	Ogre::Timer ti;

	Load();
	LoadAllSurfaces();

	scn->sc->pFluidsXml = scn->data->fluids;
	scn->sc->pReverbsXml = scn->data->reverbs;

	LogO(String(":::* Time load xmls: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");


	//  🖼️ prv tex  todo
	// int k=1024;
	// prvView.Create(k,k,"PrvView");
	// prvRoad.Create(k,k,"PrvRoad");
	//  prvTer.Create(k,k,"PrvTer");

	/*scn->roadDens.Create(k+1,k+1,"RoadDens");
	
	///  ter lay tex
	for (int i=0; i < 6; ++i)
	{	String si = toStr(i);
		scn->texLayD[i].SetName("layD"+si);
		scn->texLayN[i].SetName("layN"+si);
	}*/


	//  🎥 camera
	asp = float(mWindow->getWidth()) / float(mWindow->getHeight());
	mCamera->setNearClipDistance(0.1f);
	mCamera->setFarClipDistance(pSet->view_distance);
	mCamera->setLodBias(pSet->lod_bias);

	//  set last cam pos
	mCamera->setPosition(Vector3(pSet->cam_x, pSet->cam_y, pSet->cam_z));
	mCamera->setDirection(Vector3(pSet->cam_dx, pSet->cam_dy, pSet->cam_dz).normalisedCopy());
	// mViewport->setVisibilityMask(RV_MaskAll);  // hide prv cam rect


	//  🚧 cursors
	CreateCursors();
	TerCircleInit();


	//  🎛️ Gui  * * *
	if (pSet->startInMain)
		pSet->bMain = true;
		
	bGuiFocus = false/*true*/;  bMoveCam = true;  //*--

	//  Gui Init
	InitAppGui();
	gcom->CreateFonts();
	baseInitGui();
	gui->InitGui();
	

	mGraphicsSystem->mWorkspace = SetupCompositor();
	// mCamera->setFarClipDistance( pSet->view_distance );
	// mCamera->setLodBias( pSet->lod_bias );

	createBrushPrv();


	///  🧰  All  #if 0  in Release !!!
	///  _Tool_ scene  ...................
	#if 0
	gui->ToolSceneXml();
	exit(0);
	#endif
	
	///  _Tool_ warnings  ................
	///  takes some time
	#if 0
	gui->ToolTracksWarnings();
	exit(0);
	#endif
	
	///  🧰 _Tool_ brushes prv  .............
	#if 0
	gui->ToolBrushesPrv();
	#endif
		

	//  🏞️ Load Track
	if (pSet->autostart)
		LoadTrack();

	if (!pSet->autostart)
	{	bGuiFocus = true;
		UpdVisGui();
	}
	iObjTNew = 0;
	//SetObjNewType(0);  //?white


	gui->chkInputBar(0);  // upd vis
	gui->chkCamPos(0);
	gui->chkFps(0);
}

void App::destroyScene()
{
	scn->destroyScene();

	if (mGui)  {
		mGui->shutdown();	delete mGui;	mGui = 0;  }
	if (mPlatform)  {
		mPlatform->shutdown();	delete mPlatform;	mPlatform = 0;  }
}


//---------------------------------------------------------------------------------------------------------------
///  Load Track
//---------------------------------------------------------------------------------------------------------------

//  💥 Destroy all
void App::NewCommon(bool onlyTerVeget)
{
	if (!onlyTerVeget)
		scn->DestroyAllAtmo();  // 🌦️

	//  🌳🪨  Vegetation
	scn->DelRoadDens();
	scn->grass->Destroy();  // 🌿
	scn->DestroyTrees();

	// mSceneMgr->destroyAllStaticGeometry();
	
	if (!onlyTerVeget)
	{
		DestroyObjects(true);  // 📦
		scn->DestroyFluids();  // 💧
		scn->DestroyEmitters(true);  // 🔥
	
		scn->DestroyTerrain();  // ⛰️
	}
		
	//world.Clear();

	if (resTrk != "")  ResourceGroupManager::getSingleton().removeResourceLocation(resTrk);
	LogO("------  Loading track: "+pSet->gui.track);
	resTrk = gcom->TrkDir() + "objects";
	ResourceGroupManager::getSingleton().addResourceLocation(resTrk, "FileSystem");

	if (!onlyTerVeget)
		MinimizeMemory();
}

///  Load
//---------------------------------------------------------------------------------------------------------------
void App::LoadTrack()
{
	eTrkEvent = TE_Load;
	gui->Status("#{Loading}...", 0.3,0.6,1.0);
}
void App::LoadTrackEv()
{
	Ogre::Timer ti;
	NewCommon(false);  // full destroy
	iObjCur = -1;  iEmtCur = -1;

	scn->DestroyRoads();
	scn->DestroyPace();
	

	//  🏞️ load scene
	scn->sc->LoadXml(gcom->TrkDir()+"scene.xml");
	
	//  water RTT recreate
	// scn->UpdateWaterRTT(mCamera);
	
	BltWorldInit();

	UpdWndTitle();


	//  ⛅ sun, fog, weather, sky
	scn->CreateAllAtmo();

	scn->CreateEmitters();  // 🔥

	//  💧 Fluids
	scn->CreateFluids();


	//  set sky tex name for water
	// sh::MaterialInstance* m = mFactory->getMaterialInstance(scn->sc->skyMtr);
	// std::string skyTex = sh::retrieveValue<sh::StringValue>(m->getProperty("texture"), 0).get();
	// sh::Factory::getInstance().setTextureAlias("SkyReflection", skyTex);
	// sh::Factory::getInstance().setTextureAlias("CubeReflection", "ReflectionCube");


	//  ⛰️ Terrain
	bNewHmap = false;/**/
	scn->CreateTerrain(bNewHmap);


	//  🛣️ Road ~
	CreateRoads();

	
	//  🚦 pace ~ ~
	scn->pace = new PaceNotes(pSet);
	scn->pace->Setup(mSceneMgr, mCamera, scn->terrain, gui->mGui, mWindow);
	
	
	//  📦 Objects
	CreateObjects();
	
	//  🌳🪨 Vegetation
	if (pSet->bTrees)
	{
		scn->LoadRoadDens();
		scn->CreateTrees();  // trees after objects so they aren't inside them
		scn->grass->Create();  // 🌿
	}


	//  updates after load
	//--------------------------
	gcom->ReadTrkStats();
	gui->SetGuiFromXmls();  ///
	
	Rnd2TexSetup();
	//UpdVisGui();
	UpdStartPos();
	UpdEditWnds();  //

	/*try
	{	TexturePtr tex = TextureManager::getSingleton().getByName("waterDepth.png");
		if (tex)
			tex->reload();
	}catch(...)
	{	}*/

	gui->Status("#{Loaded}", 0.5,0.7,1.0);
	
	if (pSet->check_load)
		gui->WarningsCheck(scn->sc, scn->road);

	LogO(String(":::> Time Load Track: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}


void App::CreateRoads()  // 🛣️
{
	strlist lr;  string path = gcom->TrkDir();
	PATHMANAGER::DirList(path, lr, "xml");
	
	for (auto fname:lr)
	if (StringUtil::startsWith(fname,"road"))
	{
		int id = scn->roads.size();
		LogO("~~~R create road " + toStr(id) + " from: " + fname);
		scn->road = new SplineRoad(this);
		scn->road->Setup(
			"sphere.mesh",
			// "sphere1k.mesh", // todo: hq lods
			pSet->road_sphr, scn->terrain, mSceneMgr, mCamera, id);
		scn->road->LoadFile(path + fname);
		scn->roads.push_back(scn->road);
	}
	scn->rdCur = 0;
	scn->road = scn->roads[scn->rdCur];
}


///  🔁 Update
//---------------------------------------------------------------------------------------------------------------
void App::UpdateTrack()
{
	eTrkEvent = TE_Update;
	gui->Status("#{Updating}...", 0.2,1.0,0.5);
}
void App::UpdateTrackEv()
{
	if (!bNewHmap)
		scn->copyTerHmap();

	NewCommon(true);  // destroy only terrain and veget
	
	//CreateFluids();
	scn->CreateTerrain(bNewHmap,true);/**/

	//  road ~
	for (auto r : scn->roads)
	{
		r->mTerrain = scn->terrain;
		r->Rebuild(true);  // 🛣️
	}

	//CreateObjects();

	if (pSet->bTrees)
	{	scn->CreateTrees();
		scn->grass->Create();  // 🌿
	}
	Rnd2TexSetup();

	gui->Status("#{Updated}", 0.5,1.0,0.7);
}

//  Update btns  ---
void CGui::btnUpdateLayers(WP)
{
	if (!app->bNewHmap)
		app->scn->copyTerHmap();
	//? if (app->ndSky)
	// 	app->mSceneMgr->destroySceneNode(app->ndSky);
	app->scn->DestroyTerrain();

	app->scn->CreateTerrain(app->bNewHmap,true);
	scn->road->mTerrain = scn->terrain;
	// app->scn->updGrsTer();
}

void CGui::btnUpdateGrass(WP)
{
	scn->grass->Destroy();
	if (pSet->bTrees)
		scn->grass->Create();  // 🌿
}

void CGui::btnUpdateVeget(WP)
{
	scn->DestroyTrees();  // 🌳🪨
	if (pSet->bTrees)
		scn->CreateTrees();
}


///  📄 Save
//---------------------------------------------------------------------------------------------------------------
void App::SaveTrack()
{
	if (!pSet->allow_save)  // can force it when in writable location
	if (!pSet->gui.track_user)
	{	MyGUI::Message::createMessageBox(
			"Message", TR("#{Track} - #{RplSave}"), TR("#{CantSaveOrgTrack}"),
			MyGUI::MessageBoxStyle::IconWarning | MyGUI::MessageBoxStyle::Ok);
		return;
	}
	eTrkEvent = TE_Save;
	gui->Status("#{Saving}...", 1,0.4,0.1);

	if (pSet->check_save)
		gui->WarningsCheck(scn->sc, scn->road);
}
void App::SaveTrackEv()
{
	String dir = gcom->TrkDir();
	//  track dir in user
	gui->CreateDir(dir);
	gui->CreateDir(dir+"/objects");

	/*if (scn->terrain)
	{	float *fHmap = scn->terrain->getHeightData();
		int size = scn->sc->td.iVertsX * scn->sc->td.iVertsY * sizeof(float);

		String file = dir+"heightmap.f32";
		std::ofstream of;
		of.open(file.c_str(), std::ios_base::binary);
		of.write((const char*)fHmap, size);
		of.close();
	}*/

	int i = 0;  // all roads
	for (auto r : scn->roads)
	{
		auto si = i==0 ? "" : toStr(i+1);  ++i;
		r->SaveFile(dir+ "road"+si+".xml");
	}

	scn->sc->SaveXml(dir+"scene.xml");

	SaveGrassDens();
	SaveWaterDepth();

	gui->Delete(gui->getHMapNew());
	gui->Status("#{Saved}", 1,0.6,0.2);
}


///  ⛰️📍  Ter Circle  edit cursor   o
//-------------------------------------------------------------------------------------
const int divs = 90;
const Real aAdd = 2*PI_d / (divs), dTc = 4.f / (divs);
static Real fTcos[divs+4], fTsin[divs+4];

const static String csTerC[ED_Filter+1] = {
	"circle_deform", "circle_smooth", "circle_height", "circle_filter"};


void App::TerCircleInit()
{
	for (int d = 0; d < divs+2; ++d)
	{
		Real a = d * aAdd;
		fTcos[d] = cosf(a);  fTsin[d] = sinf(a);
	}
}

//  ⛰️📍💫  Update mesh  o
//-------------------------------------------------------------------------------------
void App::TerCircleUpd(float dt)
{
	if (!scn->terrain || !scn->road)  return;

	int e = edMode;
	bool ed = scn->road->bHitTer && bEdit();
	// road->ndHit->setVisible(ed);
	auto& nd = ndTerC[e];
	auto& mo = moTerC[e];
	
	//  upd vis all
	for (int i=0; i <= ED_Filter; ++i)
	if (ndTerC[i])
		ndTerC[i]->setVisible(e == i && ed);
	
	bool edTer = e <= ED_Filter && ed;
	if (!edTer)  return;
	
	Real radius = mBrSize[curBr] * 0.5f * scn->sc->td.fTriangleSize * 0.8f;  // par-
	//  scale with distance, to be same on screen
	Real scale = std::min(0.1f, 0.001f * scn->road->fHitDist ) * 8.f;  // par
	scn->road->ndHit->setScale(Vector3::UNIT_SCALE * scale * 0.8f * pSet->road_sphr);

	if (!mo)
	{	mo = new HudRenderable(csTerC[e], mSceneMgr,
			OT_TRIANGLE_LIST, true, false, RV_Hud,RQG_Hud2, divs, true);
		SetTexWrap(HLMS_UNLIT, csTerC[e]);
	}
	
	//  anim tc rot
	static Real ani = 0.f;
	ani += 0.2f * dt;
	if (ani > 40*M_PI)  ani -= 40*M_PI;

	mo->begin();
	for (int k = 0; k < 4; ++k)
	for (int d = 0; d < divs; ++d)
	{
		Real r = (d % 2 == 0 ? radius : radius - scale);
		Real x = r * fTcos[d] + scn->road->posHit.x,
			 z = r * fTsin[d] + scn->road->posHit.z;

		Vector3 p(x, 0.f, z);
		scn->terrain->getHeightAt(p);
		p.y += 0.3f;
		mo->position(p.x, p.y, p.z);
		mo->texUV( (d/2 * dTc + ani)*2, d % 2);
	}
	mo->end();
 
	if (!nd)
	{	nd = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		nd->attachObject(mo);  nd->setVisible(true);
	}
}


//---------------------------------------------------------------------------------------------------------------
///  🎳🆕 Bullet world
//---------------------------------------------------------------------------------------------------------------
void App::BltWorldInit()
{
	if (world)  // create world once
		return;
		//BltWorldDestroy();

	//  setup bullet world
	config = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(config);

	btScalar ws = 5000;  // world size
	broadphase = new bt32BitAxisSweep3(btVector3(-ws,-ws,-ws), btVector3(ws,ws,ws));
	solver = new btSequentialImpulseConstraintSolver();
	world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, config);

	world->setGravity(btVector3(0.0, 0.0, -9.81)); ///~
	world->getSolverInfo().m_restitution = 0.0f;
	world->getDispatchInfo().m_enableSPU = true;
	world->setForceUpdateAllAabbs(false);  //+
}

//  🎳💥 Destroy
void App::BltWorldDestroy()
{
	BltClear();

	delete world;  delete solver;
	delete broadphase;  delete dispatcher;  delete config;
}

//  🎳💥 Clear - delete bullet pointers
void App::BltClear()
{
	if (world)
	for(int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		world->removeCollisionObject(obj);

		ShapeData* sd = (ShapeData*)obj->getUserPointer();
		delete sd;  delete obj;
	}
	
	/*for (int i = 0; i < shapes.size(); i++)
	{
		btCollisionShape * shape = shapes[i];
		if (shape->isCompound())
		{
			btCompoundShape * cs = (btCompoundShape *)shape;
			for (int i = 0; i < cs->getNumChildShapes(); i++)
			{
				delete cs->getChildShape(i);
			}
		}
		delete shape;
	}
	shapes.resize(0);
	
	for(int i = 0; i < meshes.size(); i++)
	{
		delete meshes[i];
	}
	meshes.resize(0);
	
	for(int i = 0; i < actions.size(); i++)
	{
		world->removeAction(actions[i]);
	}
	actions.resize(0);*/
}


//  🎳💫 update (simulate)
//-------------------------------------------------------------------------------------
void App::BltUpdate(float dt)
{
	if (!world)  return;
	
	///  Simulate
	//double fixedTimestep(1.0/60.0);  int maxSubsteps(7);
	double fixedTimestep(1.0/160.0);  int maxSubsteps(24);  // same in game
	world->stepSimulation(dt, maxSubsteps, fixedTimestep);
	
	///  objects - dynamic (props)  -------------------------------------------------------------
	for (int i=0; i < scn->sc->objects.size(); ++i)
	{
		Object& o = scn->sc->objects[i];
		if (o.ms)
		{
			btTransform tr, ofs;
			o.ms->getWorldTransform(tr);
			/*const*/ btVector3& p = tr.getOrigin();
			const btQuaternion& q = tr.getRotation();
			o.pos[0] = p.x();  o.pos[1] = p.y();  o.pos[2] = p.z();
			o.rot[0] = q.x();  o.rot[1] = q.y();  o.rot[2] = q.z();  o.rot[3] = q.w();
			o.SetFromBlt();

			//p.setX(p.getX()+0.01f);  // move test-
			//tr.setOrigin(p);
			//o.ms->setWorldTransform(tr);
		}
	}
 }
 
