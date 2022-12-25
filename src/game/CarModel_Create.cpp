#include "pch.h"
#include "Def_Str.h"
#include "CScene.h"
#include "pathmanager.h"
#include "mathvector.h"
#include "game.h"
#include "SceneXml.h"
#include "RenderConst.h"
#include "CGame.h"
#include "CGui.h"
#include "CarModel.h"
#include "FollowCamera.h"
// #include "gameclient.hpp"

#include <Ogre.h>
// #include <OgreVector3.h>
// #include <OgreException.h>
// #include <OgreEntity.h>
#include <OgreItem.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>

// #include <OgreManualObject.h>
#include <OgreSubMesh2.h>
#include <OgreMesh2.h>

// #include <OgreMaterialManager.h>
// #include <OgreResourceGroupManager.h>

// #include <OgreParticleSystem.h>
// #include <OgreParticleEmitter.h>
// #include <OgreParticleAffector.h>
// #include <OgreRibbonTrail.h>
// #include <OgreBillboardSet.h>
// #include <OgreBillboardChain.h>

// #include <OgreHlmsPbs.h>
#include <OgreHlmsPbsDatablock.h>
// #include <OgreMaterialManager.h>

// #include <MyGUI_Gui.h>
// #include <MyGUI_TextBox.h>
using namespace Ogre;
using namespace std;
#define  FileExists(s)  PATHMANAGER::FileExists(s)


//  💥 Destroy
//------------------------------------------------------------------------------------------------------
void CarModel::Destroy()
{
	// if (pNickTxt){  pApp->mGui->destroyWidget(pNickTxt);  pNickTxt = 0;  }
	// itNextChk = 0;  ndNextChk = 0;
	// pMainNode =0;  ndSph =0;

	// delete pReflect;  pReflect = 0;
	delete fCam;  fCam = 0;

	int i,w,s;  //  trails
	/*for (w=0; w < numWheels; ++w)  if (whTrail[w]) {  whTemp[w] = 0.f;
		whTrail[w]->setVisible(false);	whTrail[w]->setInitialColour(0, 0.5,0.5,0.5, 0);
		mSceneMgr->v1::destroyMovableObject(whTrail[w]);  }*/

	//  destroy cloned materials
	// for (i=0; i < NumMaterials; ++i)
	// 	if (MaterialManager::getSingleton().resourceExists(sMtr[i]))
	// 		MaterialManager::getSingleton().remove(sMtr[i], resGrpId);
	
	for (auto l:lights)
		mSceneMgr->destroyLight(l);
	lights.clear();

	s = vDelIt.size();
	for (i=0; i < s; ++i)  mSceneMgr->destroyItem(vDelIt[i]);
	vDelIt.clear();
	
	s = vDelPar.size();
	for (i=0; i < s; ++i)  mSceneMgr->destroyParticleSystem(vDelPar[i]);
	vDelPar.clear();
	
	s = vDelNd.size();
	for (i=0; i < s; ++i)  mSceneMgr->destroySceneNode(vDelNd[i]);
	vDelNd.clear();

	if (brakes)  mSceneMgr->destroyBillboardSet(brakes);
	brakes = 0;

	//  destroy resource group, will also destroy all resources in it
	if (ResourceGroupManager::getSingleton().resourceGroupExists(resGrpId))
	 	ResourceGroupManager::getSingleton().destroyResourceGroup(resGrpId);
}

CarModel::~CarModel()
{
	Destroy();
}

void CarModel::ToDel(SceneNode* nd){		vDelNd.push_back(nd);  }
void CarModel::ToDel(Item* it){				vDelIt.push_back(it);  }
void CarModel::ToDel(ParticleSystem* par){	vDelPar.push_back(par);  }

	
//  log mesh stats
void CarModel::LogMeshInfo(const Item* ent, const String& name, int mul)
{
	const MeshPtr& msh = ent->getMesh();
	int tris=0, subs = msh->getNumSubMeshes();
	for (int i=0; i < subs; ++i)
	{
		SubMesh* sm = msh->getSubMesh(i);
		tris += sm->mVao[0][0]->getPrimitiveCount();
	}
	all_tris += tris * mul;  //wheels x4
	all_subs += subs * mul;
	LogO("MESH info:  "+name+"\t sub: "+toStr(subs)+"  tri: "+fToStr(tris/1000.f,1,4)+"k");
}


//  🆕 CreatePart mesh
//-------------------------------------------------------------------------------------------------------
void CarModel::CreatePart(SceneNode* ndCar, Vector3 vPofs,
	String sCar2, String sCarI, String sMesh, String sEnt,
	bool ghost, uint32 visFlags,
	Aabb* bbox, bool bLogInfo, bool body)
{
	if (!FileExists(sCar2 + sMesh))
		return;
	LogO("CreatePart " + sCarI + sEnt + " " + sDirname +  sMesh + " r "+  sCarI);
	
	Item *item =0;
	try
	{	item = mSceneMgr->createItem( sDirname + sMesh, sCarI, SCENE_DYNAMIC );
		pApp->scn->SetTexWrap(item);

		//**  set reflection cube
		assert( dynamic_cast<HlmsPbsDatablock *>( item->getSubItem(0)->getDatablock() ) );
		HlmsPbsDatablock *pDb =
			static_cast<HlmsPbsDatablock *>( item->getSubItem(0)->getDatablock() );
		if (!body)
			pDb->setTexture( PBSM_REFLECTION, pApp->mCubemapReflTex );
		else
		{	//  clone,  set car color
			static int id = 0;  ++id;
			db = static_cast<HlmsPbsDatablock *>(
				pDb->clone( "CarBody" + sCarI + toStr(id) ) );
			db->setTexture( PBSM_REFLECTION, pApp->mCubemapReflTex );
			item->getSubItem(0)->setDatablock( db );
			ChangeClr();
		}
	}
	catch (Ogre::Exception ex)
	{
		LogO(String("## CreatePart exc! ") + ex.what());
	}
	ToDel(item);

	if (bbox)  *bbox = item->getLocalAabb(); //getBoundingBox();
	// if (ghost)  {  item->setRenderQueueGroup(RQG_CarGhost);  item->setCastShadows(false);  }
	else  if (visFlags == RV_CarGlass)  item->setRenderQueueGroup(RQG_CarGlass);
	ndCar->attachObject(item);  item->setVisibilityFlags(visFlags);
	if (bLogInfo)  LogMeshInfo(item, sDirname + sMesh);
}


//-------------------------------------------------------------------------------------------------------
//  🆕 Create
//-------------------------------------------------------------------------------------------------------
void CarModel::Create()
{
	//if (!pCar)  return;

	String strI = toStr(iIndex)+ (cType == CT_TRACK ? "Z" : (cType == CT_GHOST2 ? "V" :""));
	mtrId = strI;
	String sCarI = "Car" + strI;
	resGrpId = sCarI;

	String sCars = PATHMANAGER::Cars() + "/" + sDirname;
	resCar = sCars + "/textures";
	String rCar = resCar + "/" + sDirname;
	String sCar = sCars + "/" + sDirname;
	
	bool ghost = isGhost();  //1 || for ghost test
	bool bLogInfo = !isGhost();  // log mesh info
	bool ghostTrk = isGhostTrk();
	

	//  Resource locations -----------------------------------------
	/// Add a resource group for this car
	ResourceGroupManager& resMgr = ResourceGroupManager::getSingleton();
	resMgr.createResourceGroup(resGrpId);
	resMgr.addResourceLocation(sCars, "FileSystem", resGrpId);
	resMgr.addResourceLocation(sCars + "/textures", "FileSystem", resGrpId);

	resMgr.initialiseResourceGroup(resGrpId, true);
	resMgr.loadResourceGroup(resGrpId);


	SceneNode* ndRoot = mSceneMgr->getRootSceneNode();
	pMainNode = ndRoot->createChildSceneNode();  ToDel(pMainNode);
	SceneNode* ndCar = pMainNode->createChildSceneNode();  ToDel(ndCar);

	//  🎥 --------  Follow Camera   --------
	if (mCamera && pCar)
	{
		fCam = new FollowCamera(mCamera, pSet);
		fCam->chassis = pCar->dynamics.chassis;
		fCam->loadCameras();
		
		//  set in-car camera position to driver position
		for (auto cam : fCam->mCameraAngles)
		{
			cam->mDist *= camDist;
			if (cam->mName == "Car driver")
				cam->mOffset = Vector3(driver_view[0], driver_view[2], -driver_view[1]);
			else if (cam->mName == "Car bonnet")
				cam->mOffset = Vector3(hood_view[0], hood_view[2], -hood_view[1]);
			else if (cam->mName == "Car ground")
				cam->mOffset = Vector3(ground_view[0], ground_view[2], -ground_view[1]);
	}	}
	

	//  📍 next checkpoint marker beam
	bool deny = pApp->gui->pChall && !pApp->gui->pChall->chk_beam;
	if (cType == CT_LOCAL && !deny && !pApp->bHideHudBeam)
	{
		itNextChk = mSceneMgr->createItem("check.mesh");  ToDel(itNextChk);
		itNextChk->setRenderQueueGroup(RQG_Weather);  itNextChk->setCastShadows(false);
		itNextChk->setDatablockOrMaterialName("checkpoint_normal");
		
		ndNextChk = ndRoot->createChildSceneNode();  ToDel(ndNextChk);
		ndNextChk->attachObject(itNextChk);  itNextChk->setVisibilityFlags(RV_Hud);
		ndNextChk->setVisible(false);
	}


	///()  grass sphere test
	/*#if 0
	Entity* es = mSceneMgr->createItem("sphere.mesh");  ToDel(es);
	es->setRenderQueueGroup(RQG_CarGhost);
	MaterialPtr mtr = MaterialManager::getSingleton().getByName("pipeGlass");
	es->setMaterial(mtr);
	ndSph = ndRoot->createChildSceneNode();  ToDel(ndSph);
	ndSph->attachObject(es);
	#endif*/

	
	//  var
	Vector3 vPofs(0,0,0);
	Aabb bodyBox;
	all_subs=0;  all_tris=0;  //stats
	
	if (bRotFix)
		ndCar->setOrientation(
			Quaternion(Degree(90),Vector3::UNIT_Y) *
			Quaternion(Degree(180),Vector3::UNIT_X));


	///  🆕 Create Models:  body, interior, glass
	//-------------------------------------------------
	const String& res = resGrpId;
	CreatePart(ndCar, vPofs, sCar, res, "_body.mesh",     "",  ghost, RV_Car,  &bodyBox,  bLogInfo, true);

	vPofs = Vector3(interiorOffset[0],interiorOffset[1],interiorOffset[2]);  //x+ back y+ down z+ right
	// if (!ghostTrk)  //!ghost)
	CreatePart(ndCar, vPofs, sCar, res, "_interior.mesh", "i", ghost, RV_Car,      0, bLogInfo);

	vPofs = Vector3::ZERO;
	CreatePart(ndCar, vPofs, sCar, res, "_glass.mesh",    "g", ghost, RV_CarGlass, 0, bLogInfo);
	

	if (vType == V_Sphere)  //; par temp-
		ndCar->setScale(2.f,2.f,2.f);


	//  💡 car lights **  // par set..
	if (0 && !ghost)
	for (int i=0; i < 2; ++i)
	{
		Light* light = mSceneMgr->createLight();
		SceneNode* lightNode = ndCar->createChildSceneNode( SCENE_DYNAMIC,
			bRotFix ?
			Vector3(i ? -0.6 : 0.6, 0.0, 1.1) :  //par
			Vector3(-1.1, 0.0, i ? -0.6 : 0.6) );
		lightNode->attachObject( light );
		light->setDiffuseColour( 1.f, 1.1f, 1.1f );
		light->setSpecularColour( 1.f, 1.1f, 1.1f );
		light->setPowerScale( Math::PI * 3 );
		light->setType( Light::LT_SPOTLIGHT );
		light->setDirection(
			bRotFix ? Vector3( 0, -0.1, 1 ) : Vector3( -1, 0.1, 0 ) );
		light->setAttenuationBasedOnRadius( 30.0f, 0.01f );
		light->setSpotlightRange(Degree(5), Degree(40), 1.0f );  //par 5 30
		light->setCastShadows(false);
		lights.push_back(light);
	}


	//  ⚫ wheels  ----------------------
	int w2 = numWheels==2 ? 1 : 2;
	for (int w=0; w < numWheels; ++w)
	{
		String siw = "Wheel" + strI + "_" + toStr(w);
		ndWh[w] = ndRoot->createChildSceneNode();  ToDel(ndWh[w]);

		String sMesh = "_wheel.mesh";  // custom
		if (w <  w2  && FileExists(sCar + "_wheel_front.mesh"))  sMesh = "_wheel_front.mesh"; else  // 2|
		if (w >= w2  && FileExists(sCar + "_wheel_rear.mesh") )  sMesh = "_wheel_rear.mesh";  else
		if (w%2 == 0 && FileExists(sCar + "_wheel_left.mesh") )  sMesh = "_wheel_left.mesh";  else  // 2-
		if (w%2 == 1 && FileExists(sCar + "_wheel_right.mesh"))  sMesh = "_wheel_right.mesh";
		
		if (FileExists(sCar + sMesh))
		{
			String name = sDirname + sMesh;
			Item* eWh = mSceneMgr->createItem(sDirname + sMesh, res);  ToDel(eWh);
			pApp->scn->SetTexWrap(eWh);
			// if (ghost)  {  eWh->setRenderQueueGroup(g);  eWh->setCastShadows(false);  }
			ndWh[w]->attachObject(eWh);  eWh->setVisibilityFlags(RV_Car);
			if (bLogInfo && (w==0 || w==2))  LogMeshInfo(eWh, name, 2);

			//**  set reflection cube
			assert( dynamic_cast<HlmsPbsDatablock *>( eWh->getSubItem( 0 )->getDatablock() ) );
			HlmsPbsDatablock *datablock =
				static_cast<HlmsPbsDatablock *>( eWh->getSubItem( 0 )->getDatablock() );
			datablock->setTexture( PBSM_REFLECTION, pApp->mCubemapReflTex );
		}		
		if (FileExists(sCar + "_brake.mesh") && !ghostTrk)
		{
			String name = sDirname + "_brake.mesh";
			Item* eBrake = mSceneMgr->createItem(name, res);  ToDel(eBrake);
			// if (ghost)  {  eBrake->setRenderQueueGroup(g);  eBrake->setCastShadows(false);  }
			ndBrake[w] = ndRoot->createChildSceneNode();  ToDel(ndBrake[w]);
			ndBrake[w]->attachObject(eBrake);  eBrake->setVisibilityFlags(RV_Car);
			if (bLogInfo && w==0)  LogMeshInfo(eBrake, name, 4);
		}
	}
	if (bLogInfo)  // all
		LogO("MESH info:  "+sDirname+"\t ALL sub: "+toStr(all_subs)+"  tri: "+fToStr(all_tris/1000.f,1,4)+"k");
	
	
	///  🔴 brake flares  ++ ++
	if (pCar)
	if (!brakePos.empty())
	{
		SceneNode* nd = ndCar->createChildSceneNode();  ToDel(nd);
		brakes = mSceneMgr->createBillboardSet();
		brakes->setDefaultDimensions(brakeSize, brakeSize);
		// brakes->setRenderQueueGroup(RQG_CarTrails);
		brakes->setVisibilityFlags(RV_Car);

		for (int i=0; i < brakePos.size(); ++i)
			brakes->createBillboard(brakePos[i], brakeClr);

		brakes->setDatablockOrMaterialName("flare1", "Popular");
		brakes->setVisible(false);
		nd->attachObject(brakes);
	}
	
	if (!ghostTrk)
	{
		//  ✨ Particles
		//-------------------------------------------------
		///  world hit sparks
		if (!parHit)
		{	parHit = mSceneMgr->createParticleSystem("Sparks");  ToDel(parHit);
			parHit->setVisibilityFlags(RV_Particles);
			
			//? MaterialPtr m = MaterialManager::getSingleton().getByName(parHit->getMaterialName());
			//? auto s = m->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getSamplerblock();
			
			SceneNode* np = ndRoot->createChildSceneNode();  ToDel(np);
			np->attachObject(parHit);
			parHit->getEmitter(0)->setEmissionRate(0);
		}
		
		///  💨 boost emitters  ------------------------
		for (int i=0; i < PAR_BOOST; ++i)
		{
			String si = strI + "_" +toStr(i);
			if (!parBoost[i])
			{	parBoost[i] = mSceneMgr->createParticleSystem(sBoostParName);  ToDel(parBoost[i]);
				parBoost[i]->setVisibilityFlags(RV_Particles);
				if (1)  // || !manualExhaustPos)
				{
					// no exhaust pos in car file, guess from bounding box
					Vector3 bsize = (bodyBox.getMaximum() - bodyBox.getMinimum())*0.5,
						bcenter = bodyBox.getMaximum() + bodyBox.getMinimum();
					//LogO("Car body bbox :  size " + toStr(bsize) + ",  center " + toStr(bcenter));
					Vector3 vp = bRotFix ?
						Vector3(bsize.z * 0.97, bsize.y * 0.65, bsize.x * 0.65 * (i==0 ? 1 : -1)) :
						Vector3(bsize.x * 0.97, bsize.y * 0.65, bsize.z * 0.65 * (i==0 ? 1 : -1));
						//Vector3(1.9 /*back*/, 0.1 /*up*/, 0.6 * (i==0 ? 1 : -1)/*sides*/
					vp.z *= boostSizeZ;
					vp += Vector3(boostOffset[0],boostOffset[1],boostOffset[2]);
					SceneNode* nb = pMainNode->createChildSceneNode(SCENE_DYNAMIC, bcenter+vp);  ToDel(nb);
					nb->attachObject(parBoost[i]);
				}else
				{	// use exhaust pos values from car file
					Vector3 pos;
					if (i==0)
						pos = Vector3(exhaustPos[0], exhaustPos[1], exhaustPos[2]);
					else if (!has2exhausts)
						continue;
					else
						pos = Vector3(exhaustPos[0], exhaustPos[1], -exhaustPos[2]);

					SceneNode* nb = pMainNode->createChildSceneNode(SCENE_DYNAMIC, pos);  ToDel(nb);
					nb->attachObject(parBoost[i]); 
				}
				parBoost[i]->getEmitter(0)->setEmissionRate(0);
		}	}

		///  💨 spaceship thrusters ^  ------------------------
		for (int w=0; w < PAR_THRUST; ++w)
		if (!sThrusterPar[w].empty())
		{
			int i2 = thrusterSizeZ[w] > 0.f ? 2 : 1;
			for (int i=0; i < i2; ++i)
			{
				int ii = w*2+i;
				String si = strI + "_" +toStr(ii);
				
				if (!parThrust[ii])
				{	parThrust[ii] = mSceneMgr->createParticleSystem(sThrusterPar[w]);  ToDel(parThrust[ii]);
					parThrust[ii]->setVisibilityFlags(RV_Particles);

					Vector3 vp = Vector3(thrusterOfs[w][0],thrusterOfs[w][1],
						thrusterOfs[w][2] + (i-1)*2*thrusterSizeZ[w]);
					SceneNode* nb = pMainNode->createChildSceneNode(SCENE_DYNAMIC, vp);  ToDel(nb);
					nb->attachObject(parThrust[ii]);
					parThrust[ii]->getEmitter(0)->setEmissionRate(0);
		}	}	}

		///  ⚫💭 wheel emitters  ------------------------
		if (!ghost)
		{
			const static String sPar[PAR_ALL] = {"Smoke","Mud","Dust","FlWater","FlMud","FlMudS"};  // for ogre name
			//  particle type names
			const String sName[PAR_ALL] = {sc->sParSmoke, sc->sParMud, sc->sParDust, "FluidWater", "FluidMud", "FluidMudSoft"};
			for (int w=0; w < numWheels; ++w)
			{
				String siw = strI + "_" +toStr(w);
				//  particles
				for (int p=0; p < PAR_ALL; ++p)
				if (!par[p][w])
				{
					par[p][w] = mSceneMgr->createParticleSystem(sName[p]);  ToDel(par[p][w]);
					par[p][w]->setVisibilityFlags(RV_Particles);
					SceneNode* np = ndRoot->createChildSceneNode();  ToDel(np);
					np->attachObject(par[p][w]);
					par[p][w]->getEmitter(0)->setEmissionRate(0.f);
				}
				//  trails
				if (!ndWhE[w])
				{	ndWhE[w] = ndRoot->createChildSceneNode();  ToDel(ndWhE[w]);  }

				if (!whTrail[w])
				{	NameValuePairList params;
					params["numberOfChains"] = "1";
					params["maxElements"] = toStr(320 * pSet->trails_len);

					whTrail[w] = (v1::RibbonTrail*)mSceneMgr->createMovableObject(
						"RibbonTrail", &mSceneMgr->_getEntityMemoryManager(SCENE_DYNAMIC), &params);
					whTrail[w]->setInitialColour(0, 0.1,0.1,0.1, 0);
					whTrail[w]->setFaceCamera(false,Vector3::UNIT_Y);
					whTrail[w]->setRenderQueueGroup(RQG_CarTrails);
					ndRoot->attachObject(whTrail[w]);
					whTrail[w]->setDatablockOrMaterialName("TireTrail", "Popular");  //?
					whTrail[w]->setCastShadows(false);
					whTrail[w]->addNode(ndWhE[w]);
				}
				whTrail[w]->setTrailLength(90 * pSet->trails_len);  //30
				whTrail[w]->setInitialColour(0, 0.1f,0.1f,0.1f, 0);
				whTrail[w]->setColourChange(0, 0.0,0.0,0.0, /*fade*/0.08f * 1.f / pSet->trails_len);
				whTrail[w]->setInitialWidth(0, 0.f);
		}	}
		UpdParsTrails();
	}
}
