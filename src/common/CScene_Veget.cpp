#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "CData.h"
#include "SceneXml.h"
#include "BltObjects.h"
#include "ShapeData.h"
#include "data/PresetsXml.h"

#include "CScene.h"
#ifdef SR_EDITOR
	#include "CApp.h"
	#include "settings.h"
#else
	#include "CGame.h"
	#include "settings.h"
	#include "game.h"
	#include "BtOgreGP.h"
#endif
#include "MersenneTwister.h"

// #include <filesystem>
#include <OgreTimer.h>
#include <Ogre.h>
#include <OgreItem.h>
#include <OgreImage2.h>
#include <OgrePixelFormatGpu.h>

#include "Terra.h"
#include <OgreCommon.h>
#include <OgreVector3.h>
using namespace Ogre;


///  🌳🪨🪴🍄  Create Vegetation
//------------------------------------------------------------------------------------------------------------------
void CScene::CreateVegets()
{
	if (ters.empty())  return;  // just gui
	LogO("C--V Create Vegetation");
	Ogre::Timer ti;
	iVegetAll = 0;
	// updGrsTer();

	//  Terrains
	//------------------------------------------------------------------------------------------
	int iVegetTer0 = 0;
	Terra* ter0 = 0;  // prev ter iHoriz 0
	for (int it = 0; it < ters.size(); ++it)
	{
		auto* ter = ters[it];
		const auto& td = sc->tds[it];
		Real tws = td.fTerWorldSize;

		SETTINGS* pSet = app->pSet;
		const auto& gs = //GameSet
		#ifdef SR_EDITOR
			pSet->gui;
		#else
			pSet->game;
		#endif

		if (td.iHorizon >= 2)  // only 1st
			continue;
		bool horiz = td.iHorizon == 1;
		if (horiz)  // less dist
		{	tws *= pSet->veg.hor_trees_dist;
			LogO("C--V Create Vegetation on Horizon");
		}

		//  density
		// Real horizMul = 1.f; // /*1.f / */ (1.f + td.iHorizon * 23);
		// if (td.iHorizon == 1)  // horiz veget only on 1st
		// 	horizMul = 

		Real dTr = sc->densTrees; // * horizMul;
		dTr = dTr / 1000000.f * tws * tws;

		Real hTr = horiz ? pSet->veg.hor_trees  : 1.f;
		Real hBu = horiz ? pSet->veg.hor_bushes : 1.f;
		//  todo: if any other terrain is here and higher, use its td.fVeget instead ..
		//  to fix trees going through from below terrain
		Real fTrees  = gs.trees  * td.fVeget * dTr * hTr;
		Real fBushes = gs.bushes * td.fVeget * dTr * hBu;
		
		if (fTrees > 0.f || fBushes > 0.f)
		{
			//if (!pSet->impostors_only)
			//sc->trDist * pSet->trees_dist, 0);
			//sc->trDistImp * pSet->trees_dist, 0);
			auto& resMgr = ResourceGroupManager::getSingleton();
			SceneManager *mgr = app->mSceneMgr;
			SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );

			const int r = imgRoadSize;  int cntshp = 0;

			//  set random seed  // add seed in scene.xml and in editor gui?
			MTRand rnd((MTRand::uint32)1213);

			TextureBox tb;
			if (imgRoad)
				tb = imgRoad->getData(0);
			else
				LogO("TREES no imgRoad !");

			///  Veget Layers  
			//------------------------------------------------------------------------------------------
			int vegAllLay = 0, nn = 0;
			for (size_t l=0; l < sc->vegLayers.size(); ++l)
			{
				VegetLayer& vg = sc->vegLayersAll[sc->vegLayers[l]];
				String file = vg.name;
				vg.cnt = 0;
		
				bool found = resMgr.resourceExistsInAnyGroup(file);
				if (!found)
				{
					LogO("WARNING: not found vegetation model: "+file);
					continue;
				}
				//  presets.xml needed
				auto nomesh = file.substr(0, file.length()-5);
				const PVeget* veg = data->pre->GetVeget(nomesh);
				if (!veg)
				{
					LogO("WARNING: not found vegetation in presets.xml: "+file);
					continue;
				}

				// Item* ent = app->mSceneMgr->createItem(file);
				// ent->setVisibilityFlags(RV_Vegetation);  ///vis+  disable in render targets
				// if (pg.windFx > 0.f)  {
				// 	trees->setCustomParam(ent->getName(), "windFactorX", pg.windFx);
				// 	trees->setCustomParam(ent->getName(), "windFactorY", pg.windFy);  }


				#if 0  // log info
				const MeshPtr& msh = ent->getMesh();
				int tris=0, subs = msh->getNumSubMeshes();
				for (int i=0; i < subs; ++i)
				{
					SubMesh* sm = msh->getSubMesh(i);
					tris += sm->indexData->indexCount;
				}
				LogO("Veget info:  "+file+"\t sub: "+toStr(subs)+"  tri: "+fToStr(tris/1000.f,1,4)+"k");
				#endif


				///  collision
				const BltCollision* col = data->objs->Find(vg.name);
				Vector3 ofs(0,0,0);  if (col)  ofs = col->offset;  // mesh offset

				//----------------------------------------------------------------
				//  models count
				//----------------------------------------------------------------
				int cnt = 6000 * vg.dens * (veg->bush ? fBushes : fTrees);  // old
				int all = 0;  // stat
				// LogO("tws: "+fToStr(tws)+" tr cnt: "+toStr(cnt)+" c/t2: "+fToStr(cnt/tws/tws,8,10));  //-
				// LogO(String("col?  ")+(col?"y":"n")+ " ofs x "+fToStr(ofs.x,2)+ " z "+fToStr(ofs.y,2));
				
				//  todo: parallel
				for (int i = 0; i < cnt; ++i)
				if (iVegetAll < pSet->veg.limit)
				{
					//  pos0 - original  pos - with offset
					Vector3 pos0 = Vector3::ZERO, pos = Vector3::ZERO;  Real yaw;

					#if 0  ///  test shapes, new objects
						yaw = (nn * 15) % 360;  // yaw = 0.f;  // grid
						pos.z = -100 +(nn / 9) * 20;
						pos.x = -100 +(nn % 9) * 20;
						Real scl = pg.minScale;  ++nn;
					#else
						yaw = rnd.rand(360.0);
						pos.x = (rnd.rand()-0.5) * tws;  //td.fTriangleSize * td.iVertsX;  //td.fTerWorldSize
						pos.z = (rnd.rand()-0.5) * tws;
						Real scl = rnd.rand() * (vg.maxScale - vg.minScale) + vg.minScale;
					#endif
					pos0 = pos;  // store original place
					bool add = true;


					//  offset mesh  pos, rotY, scl
					const float yr = Degree(yaw).valueRadians();
					const float cyr = cos(yr), syr = sin(yr);
					Vector2 vo(
						ofs.x * cyr - ofs.y * syr,  // ofs x,y for pos x,z
						ofs.x * syr + ofs.y * cyr);
					pos.x += vo.x * scl;  pos.z += vo.y * scl;


					//----------------------------------------------------------------
					//  add checks
					//----------------------------------------------------------------

					//  check for horizon if on main ter  ------------
					if (td.iHorizon > 0 && ter0)
					{
						// bool in = ter0->getHeightAt(pos);
						bool in = ter0->worldInside(pos);
						if (in)  // inside
							add = false;
					}
					if (!add)  continue;  //

					//  ter ⛰️Angle  ------------
					float ang = ter->getAngle(pos.x, pos.z, td.fTriangleSize);
					if (ang > vg.maxTerAng)
						add = false;

					// if (!add)  LogO("ter ang");
					if (!add)  continue;  //

					//  ter 🏔️Height  ------------
					// bool in = ter->worldInside(pos);
					bool in = ter->getHeightAt(pos);
					// LogO(fToStr(pos.y));
					if (!in)  add = false;  // outside
					
					if (pos.y < vg.minTerH || pos.y > vg.maxTerH)
						add = false;
					
					// if (!add)  LogO("ter h");
					if (!add)  continue;  //
					
					//  if in  🌊Fluids  ------------
					float fa = sc->GetDepthInFluids(pos);
					if (fa > vg.maxDepth)
						add = false;

					// if (!add)  LogO("in fl");

					//  if on  🛣️Road - uses roadDensity.png
					if (imgRoad && r > 0)  //  ----------------
					{
					int mx = (0.5*tws + pos.x)/tws*r,
						my = (0.5*tws + pos.z)/tws*r;

						int c = sc->trRdDist + vg.addRdist;
						int d = c;
						bool bMax = vg.maxRdist < 20; //100 slow-
						if (bMax)
							d = c + vg.maxRdist+1;  // not less than c

						//  find dist to road
						int ii,jj, rr, rmin = 3000;  //d
						for (jj = -d; jj <= d; ++jj)
						for (ii = -d; ii <= d; ++ii)
						{
							const int
								xx = std::max(0,std::min(r-1, my+ii)),
								yy = std::max(0,std::min(r-1, mx+jj));

							const float cr = tb.getColourAt(
								xx, yy, 0, Ogre::PFG_RGBA8_UNORM_SRGB ).r;
							
							if (cr < 0.75f)  //par-
							{
								rr = abs(ii)+abs(jj);
								//rr = sqrt(float(ii*ii+jj*jj));  // much slower
								rmin = std::min(rmin, rr);
							}
						}
						if (rmin <= c)
							add = false;

						if (bMax && /*d > 1 &&*/ rmin > d-1)  // max dist (optional)
							add = false;
					}
					// if (!add)  LogO("on rd");
					if (!add)  continue;  //
				// end parallel

					//  🟢 Ogre add 1
					//--------------------------------
					CreateVeget1( pos0, pos, yaw, scl,
						veg, vg, i,  horiz, col, cntshp);

					//  + count stats
					++all;  ++vg.cnt;  ++iVegetAll;  ++vegAllLay;

				}	//  i cnt  layer Models
				if (!(veg->bush && cnt == 0 && horiz))  // horiz no bushes
					LogO("Tree:  "+file+"\t cnt: "+toStr(all)+" / "+toStr(cnt));
			}	// l  veget Layers

			//  ter 0 stats
			if (horiz)
				iVegetTer0 += vegAllLay;
			if (td.iHorizon == 0 /*&& !ter0*/)  // could be many0-
				ter0 = ter;
			LogO(String("***** Vegetation models count: ") + toStr(iVegetAll) + "  shapes: " + toStr(cntshp));

		}	// > 0
	}	//  Terrains

	LogO(String(":::* Time Trees: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}


//  🟢 Ogre add
//------------------------------------------------------------------------------------------
void CScene::CreateVeget1(
	Vector3 pos0, Vector3 pos, Real yaw, Real scl,
	const PVeget* veg, const VegetLayer& vg, int i,
	bool horiz, const class BltCollision* col, int& cntshp)
{
	SETTINGS* pSet = app->pSet;
	SceneManager *mgr = app->mSceneMgr;
	SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );

	//  item
	Item *item = mgr->createItem( vg.name,
		ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, SCENE_STATIC );
	if (!vg.mtr.empty())
		item->setDatablockOrMaterialName(vg.mtr);

	item->setRenderQueueGroup( veg->alpha ? RQG_AlphaVegObj : RQG_Road );
	item->setVisibilityFlags( RV_Vegetation );
	app->SetTexWrap(item);

	//  how far visible  // todo: * norm scale, aabb?
	Real dist =  /*rare far*/ i % 3 == 0 ? veg->farDist : veg->visDist;
	Real setDist = veg->bush ?
		(/*horiz ? pSet->veg.hor_bushes_dist :*/ pSet->veg.bushes_dist) :
		(/*horiz ? pSet->veg.hor_trees_dist  :*/ pSet->veg.trees_dist);
	item->setRenderingDistance( dist * setDist );
	
	//  vec add
	vegetItems.push_back(item);
	if (!veg->bush && veg->farDist > 500)  //par..
		vegetItemsGI.push_back(item);

	#if 0  //  marker | test
		Item *item2 = mgr->createItem( "ring_blue_stick.mesh",
			ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, SCENE_STATIC );
		vegetItems.push_back(item2);

		SceneNode *node2 = rootNode->createChildSceneNode( SCENE_STATIC );
		node2->attachObject( item2 );
		node2->setPosition( pos0 );
		vegetNodes.push_back(node2);
	#endif

	//  node
	SceneNode *node = rootNode->createChildSceneNode( SCENE_STATIC );
	node->attachObject( item );
	node->scale( scl * Vector3::UNIT_SCALE );

	pos.y -= veg->yOfs;  // ofs below
	// pos.y += std::min( item->getLocalAabb().getMinimum().y, Real(0.0f) ) * -0.1f + lay.down;  //par
	// todo: ter h in few +-xz, get lowest slow-
	node->setPosition( pos );
	
	Degree a( yaw );
	Quaternion q;  q.FromAngleAxis( a, Vector3::UNIT_Y );
	if (0) // todo: par pg.tilt ..
	{
		Degree at( Math::RangeRandom(0, 20.f) );
		Quaternion qt;  qt.FromAngleAxis( -at, Vector3::UNIT_Z );
		node->setOrientation( qt * q );
	}else
		node->setOrientation( q );
	vegetNodes.push_back(node);

if (horiz)
	return;
	
	///  🎳 add to bullet world
	//----------------------------------------------------------------
#ifndef SR_EDITOR  //  in Game
	int cc = col ? col->shapes.size() : 0;
	//  not found in xml or specified, 1 shape
	bool useTrimesh = !col || cc == 1 && col->shapes[0].type == BLT_Mesh;
	bool noCol = data->objs->colNone[vg.name];

	if (pSet->game.collis_veget && !noCol)
	if (!useTrimesh)
	for (int c=0; c < cc; ++c)  // all shapes
	{
		const BltShape* shp = &col->shapes[c];
		Vector3 pos = pos0;  // restore original place
		Vector3 ofs = shp->offset;
		//  offset shape  pos, rotY, scl
		Vector2 vo;  float yr = Degree(-yaw).valueRadians();
		// LogO("veget shp  i "+toStr(i)+"  yr "+toStr(yr)+"  p "+toStr(pos0)+" ");
		float cyr = cos(yr), syr = sin(yr);
		vo.x = ofs.x * cyr - ofs.y * syr;
		vo.y = ofs.x * syr + ofs.y * cyr;
		pos.x += vo.x * scl;  pos.z += vo.y * scl;

		//  apply pos offset xyz, rotY, mul by scale
		ter->getHeightAt(pos);
		btVector3 pc(pos.x, -pos.z, pos.y + ofs.z * scl);  // center
		btTransform tr;  tr.setIdentity();  tr.setOrigin(pc);

		btCollisionShape* bshp = 0;
		if (shp->type == BLT_CapsZ)
			bshp = new btCapsuleShapeZ(shp->radius * scl, shp->height * scl);
		else
			bshp = new btSphereShape(shp->radius * scl);
		bshp->setUserPointer((void*)SU_Vegetation);

		btCollisionObject* bco = new btCollisionObject();
		bco->setActivationState(DISABLE_SIMULATION);
		bco->setCollisionShape(bshp);	bco->setWorldTransform(tr);

		if (shp->damping > 0.f)
		{
			bco->setCollisionFlags(bco->getCollisionFlags() |
				btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_NO_CONTACT_RESPONSE/**/);
			bco->setUserPointer(new ShapeData(ST_Damp, 0, 0, 0, 0, 0, shp->damping));  /// *
		}else
		{	bco->setFriction(shp->friction);  bco->setRestitution(shp->restitution);
			bco->setCollisionFlags(bco->getCollisionFlags() |
				btCollisionObject::CF_STATIC_OBJECT /*| btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);
		}
		app->pGame->collision.world->addCollisionObject(bco);
		app->pGame->collision.shapes.push_back(bshp);
		++cntshp;
	}
	else  // 🪨 use trimesh  . . . . . . . . . . . . 
	{
		const BltShape* shp = !col ? &data->objs->defPars : &col->shapes[0];
		Vector3 pc(pos0.x, pos.y, pos0.z);
		Quaternion q;  q.FromAngleAxis(Degree(yaw), Vector3::UNIT_Y);

		Matrix4 tre;  tre.makeTransform(pc, scl*Vector3::UNIT_SCALE, q);
		BtOgre::StaticMeshToShapeConverter converter(item, tre);
		btCollisionShape* shape = converter.createTrimesh();
		shape->setUserPointer((void*)SU_Vegetation);

		btCollisionObject* bco = new btCollisionObject();
		btTransform tr;  tr.setIdentity();
		bco->setActivationState(DISABLE_SIMULATION);
		bco->setCollisionShape(shape);	bco->setWorldTransform(tr);
		bco->setFriction(shp->friction);  bco->setRestitution(shp->restitution);
		bco->setCollisionFlags(bco->getCollisionFlags() |
			btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);
		app->pGame->collision.world->addCollisionObject(bco);
		app->pGame->collision.shapes.push_back(shape);
		++cntshp;
	}
	//----------------------------------------------------------------
#endif
}


void CScene::DestroyVegets()
{
	LogO("D--V destroy Veget Trees");

	SceneManager *mgr = app->mSceneMgr;
	for (auto* node : vegetNodes)
		mgr->destroySceneNode(node);
	vegetNodes.clear();
	
	for (auto* item : vegetItems)
		mgr->destroyItem(item);
	vegetItems.clear();
	vegetItemsGI.clear();
}

void CScene::RecreateVegets()
{
#ifdef SR_EDITOR
	if (!app->pSet->bTrees)
		DestroyVegets();
	else
		CreateVegets();
#else
	DestroyVegets();  // not used
	CreateVegets();
#endif
}


void CScene::LoadRoadDens()
{
	DelRoadDens();

	imgRoad = new Image2();
	try
	{	imgRoad->load(String("roadDensity.png"), "General");
		imgRoadSize = imgRoad->getWidth();  // square[]
		LogO("roadDensity: "+toStr(imgRoadSize));
	}
	catch(...)
	{	LogO("Warning: Trees can't load roadDensity !");
		DelRoadDens();
	}
}

void CScene::DelRoadDens()
{
	imgRoadSize = 0;
	delete imgRoad;  imgRoad = 0;
}
