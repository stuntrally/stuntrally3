#include "pch.h"
#include "par.h"
#include "cardefs.h"
#include "Def_Str.h"
#include "CScene.h"
#include "paths.h"
#include "mathvector.h"
#include "game.h"
#include "SceneXml.h"
#include "RenderConst.h"
#include "CGame.h"
#include "CGui.h"
#include "CarModel.h"
#include "FollowCamera.h"
// #include "gameclient.hpp"
#include "HlmsPbs2.h"

#include <OgreVector3.h>
#include <OgreColourValue.h>
#include <OgreItem.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreSubMesh2.h>
#include <OgreMesh2.h>
// #include <OgreResourceGroupManager.h>

#include <OgreParticleSystem.h>
#include <OgreParticleEmitter.h>
#include <OgreParticleAffector.h>
#include <OgreRibbonTrail.h>
#include <OgreBillboardSet.h>
// #include <OgreBillboardChain.h>

// #include <OgreHlmsPbs.h>
#include <OgreHlmsPbsDatablock.h>
// #include <OgreMaterialManager.h>

// #include <MyGUI_Gui.h>
// #include <MyGUI_TextBox.h>
using namespace Ogre;
using namespace std;
#define  FileExists(s)  PATHS::FileExists(s)


//  💥 Destroy
//------------------------------------------------------------------------------------------------------
void CarModel::Destroy()
{
	if (pNickTxt){  pApp->mGui->destroyWidget(pNickTxt);  pNickTxt = 0;  }
	// itNextChk = 0;  ndNextChk = 0;
	// pMainNode =0;  ndSph =0;

	// delete pReflect;  pReflect = 0;
	delete fCam;  fCam = 0;

	int i,w,s;  //  trails
	/*for (w=0; w < numWheels; ++w)  if (whTrail[w]) {  whTemp[w] = 0.f;
		whTrail[w]->setVisible(false);	whTrail[w]->setInitialColour(0, 0.5,0.5,0.5, 0);
		mSceneMgr->v1::destroyMovableObject(whTrail[w]);  }*/

	for (auto& l : lights)
	{	mSceneMgr->destroyLight(l.li);  l.li = 0;  }
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

	if (bsBrakes)  mSceneMgr->destroyBillboardSet(bsBrakes);
	bsBrakes = 0;
	if (bsFlares)  mSceneMgr->destroyBillboardSet(bsFlares);
	bsFlares = 0;
	if (bsReverse)  mSceneMgr->destroyBillboardSet(bsReverse);
	bsReverse = 0;

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
	String sCar2, String sCarI, String sMesh, String sMat,
	bool ghost, uint32 visFlags,
	Aabb* bbox, bool bLogInfo, bool body)
{
	auto mesh = sCar2 + sMesh;
	if (!FileExists(mesh))
		return;
	LogO("C==C CreatePart " + sCarI + " " + mesh + " r "+  sCarI + " mtr " + sMat);
	
	Item *item =0;
	try
	{	item = mSceneMgr->createItem( mesh, sCarI, SCENE_DYNAMIC );
		pApp->SetTexWrap(item);

		//**  set reflection cube
		if (pApp->mCubeReflTex)
		{
			if (!body)
			{
				assert( dynamic_cast<HlmsPbsDatablock *>( item->getSubItem(0)->getDatablock() ) );
				HlmsPbsDatablock* pDb = static_cast<HlmsPbsDatablock *>( item->getSubItem(0)->getDatablock() );
				pDb->setTexture( PBSM_REFLECTION, pApp->mCubeReflTex );
			}else
			{	//  body_paint db2
			#if 1
				// LogO("CAR MAT: "+sDirname+"_"+sMat);
				item->setDatablockOrMaterialName(sDirname+"_"+sMat);
				
				HlmsPbsDb2* pDb = dynamic_cast<HlmsPbsDb2*>( item->getSubItem(0)->getDatablock() );
				LogO(pDb ? "db2 cast ok" : "db2 cast fail");
				if (pDb)
				{	db = pDb;
					db->setTexture( PBSM_REFLECTION, pApp->mCubeReflTex );
					SetPaint();
				}
			#else  //-
				HlmsPbsDb2* pDb = dynamic_cast<HlmsPbsDb2*>( item->getSubItem(0)->getDatablock() );
				LogO(pDb ? "db2 cast ok" : "db2 cast fail");
				if (pDb)
				{
					//  clone,  set car color
					static int id = 0;  ++id;
				#if 1  // no clone  bad ghost, same car color
					db = pDb;
				#else  // fixme  clone bad-
					String s = "CarBody" + sCarI + toStr(id);
					db = static_cast<HlmsPbsDb2*>( pDb->clone( s ) );
				#endif
					db->setTexture( PBSM_REFLECTION, pApp->mCubeReflTex );
					item->setDatablock(db);
					SetPaint();
				}
				//db->setBrdf(PbsBrdf::DefaultHasDiffuseFresnel);  // ogre 2.4 mirror
			#endif
			}
	}	}
	catch (Ogre::Exception ex)
	{
		LogO(String("## Vehicle CreatePart failed! ") + ex.what());
	}
	ToDel(item);

	if (bbox)  *bbox = item->getLocalAabb(); //getBoundingBox();
	// if (ghost)  {  item->setRenderQueueGroup(RQG_CarGhost);  item->setCastShadows(false);  }
	else  if (visFlags == RV_CarGlass)  item->setRenderQueueGroup(RQG_CarGlass);
	ndCar->attachObject(item);  item->setVisibilityFlags(visFlags);
	if (bLogInfo)  LogMeshInfo(item, sDirname + sMesh);
}

//  💡🆕 Create Light
//-------------------------------------------------------------------------------------------------------
void CarModel::CreateLight(SceneNode* ndCar, LiType type, Vector3 pos, ColourValue c, float power)
{
	const bool front = type == LI_Front, brake = type == LI_Brake, under = type == LI_Under;
	const bool sphere = vType == V_Sphere;
	const Real dirY = under ?-1.f : front ? -0.1f : -0.0f;  //par-
	const Real dirZ = under ? 0.f : front ?  1.0f : -1.0f;
	
	LogO("C**L Add light: "+strLiType[type]+"  pos "+toStr(pos));  //-
	SceneNode* node = ndCar->createChildSceneNode( SCENE_DYNAMIC, pos );
	Light* l = mSceneMgr->createLight();
	node->attachObject( l );  ToDel(node);
	
	float bright = 1.17f * pSet->bright, contrast = pSet->contrast;
	l->setDiffuseColour(  c * bright * contrast );
	l->setSpecularColour( c * bright * contrast );

	CarLight li;  // bright 💡
	if (sphere)  // O
		li.power = !front ? 1.f : 2.f;
	else
		li.power = !front ? 2.f :  // back  // par 3-
			/*under ||*/ 10.f / numLights; // front
	li.power *= Math::PI * power;
	l->setPowerScale( li.power * pSet->car_light_bright);

	auto lt = //under ? Light::LT_AREA_APPROX :  // bug ter-
		sphere ? Light::LT_POINT : Light::LT_SPOTLIGHT;
	l->setType(lt);

	if (lt == Light::LT_SPOTLIGHT)
	{
		auto dir = bRotFix ?
			Vector3( 0, dirY, dirZ ) :
			Vector3( -dirZ, -dirY, 0 );
		dir.normalise();
		l->setDirection(dir);
		// float deg = front ? 40.f : !brake ? 140.f : 70.f;  // par cfg..
		float deg = front ? 40.f : !brake ? 170.f : 120.f;  // par 5 30  wide-
		l->setSpotlightRange(Degree(0), Degree(deg), 1.0f );
	}
	if (/*sphere ||*/ front /*|| under*/)
		l->setAttenuationBasedOnRadius( 30.0f, 0.01f );
	else //if (!front)  // brakes etc
		l->setAttenuationBasedOnRadius( 3.0f, 0.1f );  // par gui..

	//  underside glow __
	if (under)
	{
		l->setPowerScale( Math::PI *2.f );
		l->setAttenuationBasedOnRadius( 10.0f, 0.1f );
		// l->setDoubleSided(1);
		l->setAffectParentNode(0);
		auto dir = bRotFix ?
			Vector3( 0, dirY, dirZ ) :
			Vector3( -dirZ, -dirY, 0 );
		l->setDirection(dir.normalisedCopy());
		/*l->setRectSize(Vector2(2.5,3.5));  // area

	    const uint32 c_areaLightsPoolId = 759384;
        TextureGpuManager *mgr = pApp->mRoot->getRenderSystem()->getTextureGpuManager();
        TextureGpu *areaTex = mgr->createOrRetrieveTexture(
            "AreaLightMask0", GpuPageOutStrategy::AlwaysKeepSystemRamCopy,
            TextureFlags::AutomaticBatching, TextureTypes::Type2D, BLANKSTRING, 0u, c_areaLightsPoolId );
        l->setTexture( areaTex );
        l->mTexLightMaskDiffuseMipStart = ( Ogre::uint16 )( 0.95f * 65535 );/**/
	}
	l->setCastShadows( front && pSet->li.front_shdw);
	l->setVisible( under || front && sc->needLights);  // auto on for dark tracks  set pCar ..

	li.li = l;  li.type = type;
	lights.push_back(li);  // add
}

//-------------------------------------------------------------------------------------------------------
//  🆕 Create
//-------------------------------------------------------------------------------------------------------
void CarModel::Create()
{
	LogO("");
	LogO("C==C ----- Creating Vehicle: "+sDirname+" -----");
	//if (!pCar)  return;

	String strI = toStr(iIndex)+ (cType == CT_TRACK ? "Z" : (cType == CT_GHOST2 ? "V" :""));
	mtrId = isGhostTrk() ? "T" : isGhost() ? "G" : toStr(iIndex);  // _ in cars.material
	String sCarI = "Car" + strI;
	resGrpId = sCarI;  // just for unique name

	String sCars = PATHS::Cars() + "/" + sDirname;
	resCar = sCars + "/textures";
	String rCar = resCar + "/" + sDirname;
	String sCar = sCars + "/" + sDirname;
	
	bool ghost = isGhost();  //1 || for ghost test
	bool bLogInfo = !isGhost();  // log mesh info
	bool ghostTrk = isGhostTrk();
	

	//  Resource locations -----------------------------------------
	/// Add a resource group for this car
	auto& resMgr = ResourceGroupManager::getSingleton();
	resMgr.createResourceGroup(resGrpId);
	resMgr.addResourceLocation(sCars, "FileSystem", resGrpId);
	resMgr.addResourceLocation(sCars + "/textures", "FileSystem", resGrpId);

	resMgr.initialiseResourceGroup(resGrpId, true);
	resMgr.loadResourceGroup(resGrpId);


	SceneNode* ndRoot = mSceneMgr->getRootSceneNode();
	ndMain = ndRoot->createChildSceneNode();  ToDel(ndMain);
	SceneNode* ndCar = ndMain->createChildSceneNode();  ToDel(ndCar);

	//  🎥 --------  Follow Camera   --------
	if (cam && pCar)
	{
		fCam = new FollowCamera(cam, pSet);
		fCam->chassis = pCar->dynamics.chassis;
		fCam->loadCameras();
	}
	//  set car type cameras offset from .car
	if (fCam)
	for (auto& c : fCam->mViews)
	{
		c.mDist *= camDist;
		if (c.mName == "Car driver")
			c.mOffset = Vector3(driver_view[0], driver_view[2], -driver_view[1]);
		else if (c.mName == "Car bonnet")
			c.mOffset = Vector3(hood_view[0], hood_view[2], -hood_view[1]);
		else if (c.mName == "Car ground")
			c.mOffset = Vector3(ground_view[0], ground_view[2], -ground_view[1]);
	}
	

	//  🥛 next checkpoint marker beam
	bool deny = pApp->gui->pChall && !pApp->gui->pChall->chk_beam;
	if (cType == CT_LOCAL && !deny && !pApp->bHideHudBeam)
	{
		itNextChk = mSceneMgr->createItem("check.mesh");  ToDel(itNextChk);
		itNextChk->setRenderQueueGroup(RQG_Ghost);  itNextChk->setCastShadows(false);
		itNextChk->setDatablockOrMaterialName("checkpoint_normal");
		
		ndNextChk = ndRoot->createChildSceneNode();  ToDel(ndNextChk);
		ndNextChk->attachObject(itNextChk);
		itNextChk->setVisibilityFlags(RV_Hud3D[iIndex]);  ndNextChk->setVisible(false);
	}


	//  🟢🌿 grass sphere test
	/*#if 0
	Item* es = mSceneMgr->createItem("sphere.mesh");  ToDel(es);
	es->setRenderQueueGroup(RQG_CarGhost);
	es->setDatablockOrMaterialName("pipeGlass");
	ndSph = ndRoot->createChildSceneNode();  ToDel(ndSph);
	ndSph->attachObject(es);
	#endif*/

	
	//  var
	Vector3 vPofs(0,0,0);
	Aabb bodyBox;
	all_subs=0;  all_tris=0;  //stats
	
	if (bRotFix)
	{	ndCar->setOrientation(
			Quaternion(Degree(90),Vector3::UNIT_Y) *
			Quaternion(Degree(180),Vector3::UNIT_X));
		// ndCar->_getFullTransformUpdated();  //at end?
	}

	///  🆕 Create Models:  body, interior, glass
	//-------------------------------------------------
	const String& res = resGrpId;
	CreatePart(ndCar, vPofs, sCar, res, "_body.mesh",     mtrId,  ghost, RV_Car,  &bodyBox,  bLogInfo, true);

	vPofs = Vector3(interiorOfs[0],interiorOfs[1],interiorOfs[2]);  //x+ back y+ down z+ right
	// if (!ghostTrk)  //!ghost)
	CreatePart(ndCar, vPofs, sCar, res, "_interior.mesh", mtrId, ghost, RV_Car,      0, bLogInfo);

	vPofs = Vector3::ZERO;
	CreatePart(ndCar, vPofs, sCar, res, "_glass.mesh",    mtrId, ghost, RV_CarGlass, 0, bLogInfo);
	
	bool sphere = vType == V_Sphere, hover = isHover();


	//  💡 car lights  ----------------------
	if (pSet->li.front && !ghost)
	{
		int cnt = fsFlares.pos.size();
		const Real dirY = -0.1f, dirZ = 1.f;  //par-
		LogO("C--L create car lights: " + toStr(numLights) + " flares: " + toStr(cnt));

		for (int i=0; i < numLights; ++i)
		if (i < cnt)
		// if (fsFlares.lit[i])
			CreateLight(ndCar, LI_Front, fsFlares.pos[i], fsFlares.clr, fsFlares.lit[i]);  //💡
	}

	if (pSet->li.under && !ghost)
	{
		ColourValue cu;
		auto c = pSet->gui.clr[0].glow;
		cu.setHSB(1.f - c.hue, c.sat, c.val);

		for (auto& pos : fsUnder.pos)
			CreateLight(ndCar, LI_Under, pos, cu, 1.f);  //💡
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
			pApp->SetTexWrap(eWh);
			// if (ghost)  {  eWh->setRenderQueueGroup(g);  eWh->setCastShadows(false);  }
			ndWh[w]->attachObject(eWh);  eWh->setVisibilityFlags(RV_Car);
			if (bLogInfo && (w==0 || w==2))  LogMeshInfo(eWh, name, 2);

			//**  set reflection cube
			if (pApp->mCubeReflTex)
			{
				assert( dynamic_cast<HlmsPbsDatablock *>( eWh->getSubItem(0)->getDatablock() ) );
				HlmsPbsDatablock* db =
					static_cast<HlmsPbsDatablock *>( eWh->getSubItem(0)->getDatablock() );
				db->setTexture( PBSM_REFLECTION, pApp->mCubeReflTex );
		}	}
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
	
	
	///  🔴 flares and lights  ++ ++
	bool hasBrakes = !fsBrakes.pos.empty(), hasReverse = !fsReverse.pos.empty(),
		 hasFlares = !fsFlares.pos.empty() && numLights > 0;
	if (pCar && (hasBrakes || hasFlares || hasReverse))
	{
		SceneNode* nd = ndCar->createChildSceneNode();  ToDel(nd);
		//  brake 
		if (hasBrakes)
		{	bsBrakes = mSceneMgr->createBillboardSet();
			auto s = fsBrakes.size;
			bsBrakes->setDefaultDimensions(s, s);
			// brakes->setRenderQueueGroup(RQG_CarTrails);
			bsBrakes->setVisibilityFlags(RV_Car);

			int n = 0;
			for (auto& pos : fsBrakes.pos)
			{
				bsBrakes->createBillboard(pos, fsBrakes.clr);
				if (fsBrakes.lit[n] && pSet->li.brake && !ghost)
					CreateLight(ndCar, LI_Brake, pos, fsBrakes.clr, fsBrakes.lit[n]);  //💡
				++n;
			}
			bsBrakes->setDatablockOrMaterialName("flare1", "Popular");
			nd->attachObject(bsBrakes);
			bsBrakes->setVisible(false);
		}
		//  front flares
		if (hasFlares && !ghost)
		{	bsFlares = mSceneMgr->createBillboardSet();
			auto s = fsFlares.size;
			bsFlares->setDefaultDimensions(s, s);
			bsFlares->setVisibilityFlags(RV_Car);

			for (auto p : fsFlares.pos)
				bsFlares->createBillboard(p, fsFlares.clr);

			bsFlares->setDatablockOrMaterialName("flare1", "Popular");
			nd->attachObject(bsFlares);
			if (pCar)  // on
			{	if (sc->needLights)
					pCar->bLightsOn = 1;
				bsFlares->setVisible(pSet->li.front && pCar->bLightsOn);
			}
		}
		//  reverse
		if (hasReverse && !ghost)
		{	bsReverse = mSceneMgr->createBillboardSet();
			auto s = fsReverse.size;
			bsReverse->setDefaultDimensions(s, s);
			bsReverse->setVisibilityFlags(RV_Car);

			int n = 0;
			for (auto& pos : fsReverse.pos)
			{
				bsReverse->createBillboard(pos, fsReverse.clr);
				if (fsReverse.lit[n] && pSet->li.reverse && !ghost)
					CreateLight(ndCar, LI_Revese, pos, fsReverse.clr, fsReverse.lit[n]);  //💡
				++n;
			}
			bsReverse->setDatablockOrMaterialName("flare1", "Popular");
			nd->attachObject(bsReverse);
			bsReverse->setVisible(false);
		}
	}
	
	if (!ghostTrk)
	{
		//  ✨ Particles
		//-------------------------------------------------
		///  world hit sparks
		if (!parHit)
		{	parHit = mSceneMgr->createParticleSystem("Sparks");  ToDel(parHit);
			parHit->setVisibilityFlags(RV_Particles);
			
			SceneNode* np = ndRoot->createChildSceneNode();  ToDel(np);
			np->attachObject(parHit);
			parHit->getEmitter(0)->setEmissionRate(0);
		}
		
		///  💨 boost emitters  ------------------------
		for (int i=0; i < boostCnt; ++i)
		{
			String si = strI + "_" +toStr(i);
			if (!parBoost[i])
			{	parBoost[i] = mSceneMgr->createParticleSystem(sBoostParName);  ToDel(parBoost[i]);
				parBoost[i]->setVisibilityFlags(RV_Particles);
				parBoost[i]->setRenderQueueGroup(RQG_CarParticles);  //RQG_CarTrails

				const Vector3& pos = boostPos[i];
				SceneNode* nb = ndCar->createChildSceneNode(SCENE_DYNAMIC, pos);  ToDel(nb);
				nb->attachObject(parBoost[i]);

				if (pSet->li.boost && !ghost)
					CreateLight(ndCar, LI_Boost, pos,
						ColourValue(boostClr[0], boostClr[1], boostClr[2]), 1.f);  //💡

				if (bRotFix)
					parBoost[i]->getEmitter(0)->setDirection(Vector3(0,0,-1));
				parBoost[i]->getEmitter(0)->setEmissionRate(0);
		}	}

		///  💨 spaceship thrusters ^  ------------------------
		for (int w=0; w < PAR_THRUST; ++w)
		{	const auto& t = thruster[w];
			if (t.sPar.empty())  continue;

			int i2 = t.sizeZ > 0.f ? 2 : 1;
			for (int i=0; i < i2; ++i)
			{
				int ii = w*2+i;
				String si = strI + "_" +toStr(ii);
				
				if (parThrust[ii])  continue;
				parThrust[ii] = mSceneMgr->createParticleSystem(t.sPar);  ToDel(parThrust[ii]);
				parThrust[ii]->setVisibilityFlags(RV_Particles);
				parThrust[i]->setRenderQueueGroup(RQG_CarParticles);

				Vector3 pos = Vector3(t.ofs[0], t.ofs[1],
					t.ofs[2] + (i-1)*2 * t.sizeZ);
					
				SceneNode* nb = ndMain->createChildSceneNode(SCENE_DYNAMIC, pos);  ToDel(nb);
				nb->attachObject(parThrust[ii]);
				parThrust[ii]->getEmitter(0)->setEmissionRate(0);

				if (t.lit && pSet->li.boost && !ghost)
					CreateLight(ndCar, LI_Thrust, pos,
						ColourValue(thrustClr[0], thrustClr[1], thrustClr[2]), 1.f);  //💡
		}	}

		///  ⚫💭 wheel emitters  ------------------------
		if (!ghost)
		{
			const static String sPar[PAR_ALL] = {
				"Smoke","Mud","Dust", "FlWater","FlMud","FlMudS"};  // for ogre name
			//  particle type names
			const String sName[PAR_ALL] = {
				sc->sParSmoke, sc->sParMud, sc->sParDust, sc->sFluidWater, sc->sFluidMud, sc->sFluidMudSoft};
			
			for (int w=0; w < numWheels; ++w)
			{
				String siw = strI + "_" +toStr(w);
				//  particles
				for (int p=0; p < PAR_ALL; ++p)
				if (!par[p][w])
				{
					par[p][w] = mSceneMgr->createParticleSystem(sName[p]);  ToDel(par[p][w]);
					par[p][w]->setVisibilityFlags(RV_Particles);
					// par[p][w]->setRenderQueueGroup(RQG_CarParticles);  // in UpdParsTrails
					SceneNode* np = ndRoot->createChildSceneNode();  ToDel(np);
					np->attachObject(par[p][w]);
					par[p][w]->getEmitter(0)->setEmissionRate(0.f);
				}
				if (hover)  continue;

				//  trails
				if (!ndWhE[w])
				{	ndWhE[w] = ndRoot->createChildSceneNode();  ToDel(ndWhE[w]);  }

				if (!whTrail[w])
				{	NameValuePairList params;
					params["numberOfChains"] = "1";
					params["maxElements"] = toStr(320 * pSet->trails_len);
					params["useVertexColours"] = "true";
					params["useTextureCoords"] = "true";

					whTrail[w] = (v1::RibbonTrail*)mSceneMgr->createMovableObject(
						"RibbonTrail", &mSceneMgr->_getEntityMemoryManager(SCENE_DYNAMIC), &params);
					whTrail[w]->setUseVertexColours(true);
					whTrail[w]->setUseTextureCoords(true);
					whTrail[w]->setInitialColour(0, 0.1,0.1,0.1, 0);
					whTrail[w]->setFaceCamera(false,Vector3::UNIT_Y);
					whTrail[w]->setRenderQueueGroup(RQG_CarTrails);
					whTrail[w]->setDatablockOrMaterialName("TireTrail", "General");  //
					whTrail[w]->setCastShadows(false);
					whTrail[w]->addNode(ndWhE[w]);
					ndRoot->attachObject(whTrail[w]);
				}
				whTrail[w]->setTrailLength(90 * pSet->trails_len);  //30
				whTrail[w]->setInitialColour(0, 0.1f,0.1f,0.1f, 0);
				whTrail[w]->setColourChange(0, 0.0,0.0,0.0, /*fade*/0.08f * 1.f / pSet->trails_len);
				whTrail[w]->setInitialWidth(0, 0.f);
		}	}
		UpdParsTrails();
	}
	LogO("C--L ----- Created Vehicle: "+sDirname+"  total lights: "+toStr(lights.size())+" -----\n");
}
