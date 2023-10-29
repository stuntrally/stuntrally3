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
#include "FluidsReflect.h"
#include "paths.h"
#include "RenderConst.h"
#include "TracksXml.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "enums.h"
#include "HudRenderable.h"
#include "GraphicsSystem.h"
#include "MainEntryPoints.h"

#include <OgreTimer.h>
#include "Terra.h"
#include "HlmsPbs2.h"
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
#include <OgreGpuResource.h>
#include <OgreResourceGroupManager.h>
#include <OgreSceneNode.h>
#include <OgreMeshManager2.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include "MessageBox.h"
#include <cmath>
using namespace Ogre;
using namespace std;


bool Args::Help()  // ❔
{
	if (has("?") || has("help"))
	{
		cout << "SR3-Editor  command line Arguments help  ----\n";
		Common("ed");
		// todo: // fixme crash ter, load hmap only?
		cout << "  sc or scene - Runs scene.xml checks for all tracks.\n";
		cout << "  warn or warnings - Runs warnings checks for all tracks.\n";
		return 1;
	}
	return 0;
}


//  🆕 Create Scene
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
	
	//  set fluids in Pbs2
	auto* mgr = mRoot->getHlmsManager();
	HlmsPbs2 *hlmsPbs2 = static_cast<HlmsPbs2 *>( mgr->getHlms( HLMS_PBS ) );
	hlmsPbs2->pFluidsXml = scn->sc->pFluidsXml;


	//  🖼️ prv tex  todo
	prvView.mgr = mSceneMgr->getDestinationRenderSystem()->getTextureGpuManager();
	int k = 1024;
	prvView.Create(k,k,"PrvView");
	prvRoad.Create(k,k,"PrvRoad");
	 prvTer.Create(k,k,"PrvTer");
	
	prvBrush.Create(BrPrvSize,BrPrvSize,"PrvBrush");
	prvBrushes.Create(2*k,2*k,"PrvBrushes");

	//scn->roadDens.Create(k+1,k+1,"RoadDens");  // var size..


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
	

	// mGraphicsSystem->mWorkspace = 
	SetupCompositor();
	// mCamera->setFarClipDistance( pSet->view_distance );
	// mCamera->setLodBias( pSet->lod_bias );


	//  🎥 camera
	asp = float(mWindow->getWidth()) / float(mWindow->getHeight());
	mCamera->setNearClipDistance(0.1f);
	mCamera->setFarClipDistance(pSet->g.view_distance);
	mCamera->setLodBias(pSet->g.lod_bias);

	//  set last cam pos
	mCamera->setPosition(Vector3(pSet->cam_x, pSet->cam_y, pSet->cam_z));
	mCamera->setDirection(Vector3(pSet->cam_dx, pSet->cam_dy, pSet->cam_dz).normalisedCopy());
	// mViewport->setVisibilityMask(RV_MaskAll);  // hide prv cam rect


	CreatePreviews();
	
	prvScene.Create(this);


	///  🧰  Command line arguments  ----
	auto& args = MainEntryPoints::args;
	// args.set("scene");  // debug

	LogO("A--- Cmd Line Arguments: "+toStr(args.all.size()));
	LogO("A--- exe path: "+args.all[0]);
	bool quit = 0;

	///  _Tool_ scene  ...................
	if (args.has("scene") || args.has("sc"))
	{	gui->ToolSceneXml();  quit = 1;  }

	///  _Tool_ warnings  ................
	// args.set("warn");  // debug
	if (args.has("warnings") || args.has("warn"))
	{	gui->ToolTracksWarnings();  quit = 1;  }
		
	if (quit)
		exit(0);  //! destroy?..


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
}


//  💥 Destroy  LoadCleanUp()
//----------------------------------------------------------------------------------
void App::destroyScene()
{
	DestroyRnd2Tex();
	prvScene.Destroy();
	// scn->destroyScene();

	LogO("DD-- destroyScene ------DD");
	
	scn->DelRoadDens();
	scn->grass->Destroy();
	scn->DestroyTrees();
	DestroyObjects(true);
	scn->DestroyRoads();
	scn->DestroyTerrains();
	scn->refl.DestroyFluids();
	scn->refl.DestroyRTT();
	scn->DestroyEmitters(true);
	scn->DestroyAllAtmo();

	// scn->DestroyTrail(0);
	scn->DestroyPace();


	DestroyGui();
	
	LogO(">>>>>>>> Destroy SR Ed done ----");

	///+  save settings
	SaveCam();
	pSet->Save(PATHS::SettingsFile(1));
	scn->data->user->SaveXml(PATHS::UserConfigDir() + "/tracks.xml");
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

	if (!onlyTerVeget)
	{
		DestroyObjects(true);  // 📦
		scn->refl.DestroyFluids();  // 💧
		scn->DestroyEmitters(true);  // 🔥
	
		scn->DestroyTerrains();  // ⛰️
	}

	DestroyRnd2Tex();  // 🖼️

	//world.Clear();

	if (resTrk != "")  ResourceGroupManager::getSingleton().removeResourceLocation(resTrk);
	LogO("------  Loading track: "+pSet->gui.track);
	resTrk = gcom->TrkDir() + "objects";
	ResourceGroupManager::getSingleton().addResourceLocation(resTrk, "FileSystem");

	if (!onlyTerVeget)
		MinimizeMemory();  // !


	//  meh, works
	for (int i=0; i<2; ++i)
		prvBrushes.Load(PATHS::UserConfigDir()+"/brushes.png",1);
}

///  Load
//---------------------------------------------------------------------------------------------------------------
void App::LoadTrack()
{
	eTrkEvent = TE_Load;  iUpdEvent = 0;
	gui->Status("#{Loading}...", 0.3,0.6,1.0);
}
void App::LoadTrackEv()
{
	Ogre::Timer ti;

	DelNewHmaps();  // F5 Load drops any new
	scn->iNewHmap = -1;  //

	NewCommon(false);  // full destroy
	iObjCur = -1;  iEmtCur = -1;

	scn->DestroyRoads();
	scn->DestroyPace();

	scn->refl.DestroyFluids();	// 💧
	scn->refl.DestroyRTT();

	// mSceneMgr->destroyAllItems();
	// mSceneMgr->destroyAllMovableObjects();
	// MeshManager::getSingleton().destroyAllResourcePools();
	// v1::MeshManager::getSingleton().destroyAllResourcePools();
	// destroyAllVertexArrayObjects();
	// mSceneMgr->_destroyAllCubemapProbes()

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
	scn->refl.CreateRTT();
	scn->refl.CreateFluids();

	//  set sky tex name for water
	// sh::MaterialInstance* m = mFactory->getMaterialInstance(scn->sc->skyMtr);
	// std::string skyTex = sh::retrieveValue<sh::StringValue>(m->getProperty("texture"), 0).get();


	//  ⛰️ Terrain
	scn->CreateTerrains(1);
	scn->TerNext(0);


	CreateRnd2Tex();  // 🖼️ RTTs after ter, size
	AddListenerRnd2Tex();


	//  🛣️ Road ~
	CreateRoads();

	
	//  🚦 pace ~ ~
	Cam* cam = &mCams[0];  // todo: lod cam-
	scn->pace[0] = new PaceNotes(pSet);
	scn->pace[0]->Setup(mSceneMgr, cam->cam, scn->ter, gui->mGui, mWindow);
	
	
	//  📦 Objects
	CreateObjects();
	
	//  🌳🪨 Vegetation
	if (pSet->bTrees)
	{
		scn->LoadRoadDens();
		scn->CreateTrees();  // trees after objects so they aren't inside them
		scn->grass->Create(this);  // 🌿
	}


	//  updates after load
	//--------------------------
	gcom->ReadTrkStats();
	gui->SetGuiFromXmls();  ///
	
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
	
	if (pSet->check_load)  // fixme
		gui->WarningsCheck(scn->sc, scn->roads);

	LogO(String(":::> Time Load Track: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}


void App::CreateRoads()  // 🛣️
{
	strlist lr;  string path = gcom->TrkDir();
	PATHS::DirList(path, lr, "xml");
	
	for (auto fname:lr)
	if (StringUtil::startsWith(fname,"road"))
	{
		int id = scn->roads.size();
		LogO("C~~R create road " + toStr(id) + " from: " + fname);
		Cam* cam = &mCams[0];  // todo: lod cam-
		scn->road = new SplineRoad(this);
		scn->road->Setup(
			"sphere.mesh",
			// "sphere1k.mesh", // todo: hq lods
			pSet->road_sphr, scn, mSceneMgr, cam, id);
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
	eTrkEvent = TE_Update;  iUpdEvent = 0;
	gui->Status("#{Updating}...", 0.2,1.0,0.5);
}
void App::UpdateTrackEv()
{
	// if (!bNewHmap)
	// 	scn->copyTerHmap();

	NewCommon(true);  // destroy only terrain and veget
	
	//CreateFluids2();
	scn->DestroyTerrains();
	scn->CreateTerrains(1);

	//  road ~
	for (auto r : scn->roads)
	{
		r->scn = scn;
		r->Rebuild(true);  // 🛣️
	}
	//CreateObjects();

	if (pSet->bTrees)
	{	scn->CreateTrees();
		scn->grass->Create(this);  // 🌿
	}
	DestroyRnd2Tex();
	CreateRnd2Tex();  // if new hmap size

	gui->Status("#{Updated}", 0.5,1.0,0.7);
}


//  Update btns  ---
void CGui::btnUpdateLayers(WP)
{
	// if (!app->bNewHmap)
	// 	app->scn->copyTerHmap();
	scn->DestroyTerrains();
	scn->CreateTerrains(1);  // 🏔️
	scn->road->scn = scn;
	scn->TerNext(0);
	// app->scn->updGrsTer();
}

void CGui::btnUpdateGrass(WP)
{
	scn->grass->Destroy();
	if (pSet->bTrees)
		scn->grass->Create(app);  // 🌿
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
	if (scn->ters.empty())  // track not loaded
		return;
	if (!pSet->allow_save)  // can force it when in writable location
	if (!pSet->gui.track_user)
	{	MyGUI::Message::createMessageBox(
			"Message", TR("#{Track} - #{RplSave}"), TR("#{CantSaveOrgTrack}"),
			MyGUI::MessageBoxStyle::IconWarning | MyGUI::MessageBoxStyle::Ok);
		return;
	}
	eTrkEvent = TE_Save;  iUpdEvent = 0;
	gui->Status("#{Saving}...", 1,0.4,0.1);

	if (pSet->check_save)
		gui->WarningsCheck(scn->sc, scn->roads);
}
void App::SaveTrackEv()
{
	String dir = gcom->TrkDir();
	//  track dir in user
	gui->CreateDir(dir);
	gui->CreateDir(dir+"/objects");

	//  all terrains, save Hmap
	int i = 0;
	for (auto ter : scn->ters)
	if (ter)  //-
	{
		auto& td = scn->sc->tds[i];
		int size = td.iVertsX;
		ter->readBackHmap(td.hfHeight, size);
		int fsize = size * size * sizeof(float);

		String file = scn->getHmap(i, false);
		std::ofstream of;
		of.open(file.c_str(), std::ios_base::binary);
		of.write((const char*)&td.hfHeight[0], fsize);
		of.close();  ++i;
	}

	i = 0;  // all roads
	for (auto r : scn->roads)
	{
		auto si = i==0 ? "" : toStr(i+1);  ++i;
		r->SaveFile(dir + "road" +si+ ".xml");
	}

	scn->sc->SaveXml(dir + "scene.xml");

	SaveGrassDens();
	// SaveWaterDepth();  //-

	DelNewHmaps();

	gui->Status("#{Saved}", 1,0.6,0.2);
}

void App::DelNewHmaps()
{
	int all = scn->ters.size() + 1;
	for (int i=0; i < all; ++i)
		gui->Delete(scn->getHmap(i, true));
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
	if (!scn->ter || !scn->road)  return;

	int e = edMode;
	bool ed = scn->road->bHitTer && bEdit();
	// road->ndHit->setVisible(ed);
	auto& nd = ndTerC[e];
	auto& mo = hrTerC[e];
	
	//  upd vis all
	for (int i=0; i <= ED_Filter; ++i)
	if (ndTerC[i])
		ndTerC[i]->setVisible(e == i && ed);
	
	bool edTer = e <= ED_Filter && ed;
	if (!edTer)  return;
	
	const Real radius = curBr().size * 0.5f *
		scn->sc->tds[scn->terCur].fTriangleSize;  // par-
	//  scale with distance, to be same on screen
	const Real dist = std::min(3000.f, scn->road->fHitDist );
	scn->road->ndHit->setScale(Vector3::UNIT_SCALE * dist * pSet->road_sphr * 0.01f);  // par
	const Real Radd = std::max(0.01f, dist * 0.02f);  // par..

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
		const Real r = (d % 2 == 0 ? radius : radius - Radd);
		const Real x = r * fTcos[d] + scn->road->posHit.x,
			z = r * fTsin[d] + scn->road->posHit.z;

		Vector3 p(x, 0.f, z);
		scn->ter->getHeightAt(p);
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
		if (body)
			delete body->getMotionState();

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
 
