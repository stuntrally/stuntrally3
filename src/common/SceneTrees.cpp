#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "CData.h"
#include "SceneXml.h"
#include "Axes.h"
#include "BltObjects.h"
#include "ShapeData.h"

#include "CScene.h"
// #include "SplineBase.h"
// #include "GuiCom.h"
#ifdef SR_EDITOR
	#include "CApp.h"
	#include "settings.h"
#else
	#include "CGame.h"
	#include "settings.h"
	#include "game.h"
	// #include "SplitScreen.h"
	#include "BtOgreGP.h"
#endif
#include "paths.h"
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


//---------------------------------------------------------------------------------------------------------------
///  🌳🪨 Vegetation  ^ ^ ^ ^
//---------------------------------------------------------------------------------------------------------------

void CScene::DestroyTrees()
{
	LogO("D--- destroy Trees");

	SceneManager *mgr = app->mSceneMgr;
	for (auto node : vegetNodes)
		mgr->destroySceneNode(node);
	vegetNodes.clear();
	
	for (auto item : vegetItems)
		mgr->destroyItem(item);
	vegetItems.clear();
}

void CScene::RecreateTrees()
{
#ifdef SR_EDITOR
	if (!app->pSet->bTrees)
		DestroyTrees();
	else
		CreateTrees();
#else
	DestroyTrees();  // not used
	CreateTrees();
#endif
}

void CScene::CreateTrees()
{
	LogO("C--- CreateTrees");
	Ogre::Timer ti;
	iVegetAll = 0;
	// updGrsTer();

	const auto* ter0 = ters[0];  // 1st ter only-
	const auto& td = sc->tds[0];  // todo: for all
	Real tws = td.fTerWorldSize * 0.5f;

	//  pos0 - original  pos - with offset
	Vector3 pos0 = Vector3::ZERO, pos = Vector3::ZERO;  Real yaw;

	SETTINGS* pSet = app->pSet;
	#ifdef SR_EDITOR
	Real fTrees = pSet->gui.trees * sc->densTrees;
	#else
	Real fTrees = pSet->game.trees * sc->densTrees;
	#endif
	

	//---------------------------------------------- Trees ----------------------------------------------
	if (fTrees > 0.f)
	{
		//if (!pSet->impostors_only)
		//sc->trDist * pSet->trees_dist, 0);
		//sc->trDistImp * pSet->trees_dist, 0);
		auto& resMgr = ResourceGroupManager::getSingleton();
		SceneManager *mgr = app->mSceneMgr;
		SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );

		tws = td.fTerWorldSize;
		const int r = imgRoadSize;  int cntshp = 0;

		//  set random seed  // add seed in scene.xml and in editor gui?
		MTRand rnd((MTRand::uint32)1213);
		#define getTerPos()		(rnd.rand()-0.5) * td.fTerWorldSize

		TextureBox tb;
		if (imgRoad)
			tb = imgRoad->getData(0);

		//  Tree Layers
		for (size_t l=0; l < sc->pgLayers.size(); ++l)
		{
			PagedLayer& pg = sc->pgLayersAll[sc->pgLayers[l]];
			String file = pg.name;  //, fpng = file+".png";
			pg.cnt = 0;

			bool found = resMgr.resourceExistsInAnyGroup(file);
			if (!found)
			{
				LogO("WARNING: not found vegetation model: "+file);
				continue;
				//file = "sphere.mesh";  // if not found, use white sphere
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
			LogO("TREE info:  "+file+"\t sub: "+toStr(subs)+"  tri: "+fToStr(tris/1000.f,1,4)+"k");
			#endif


			///  collision object
			const BltCollision* col = data->objs->Find(pg.name);
			Vector3 ofs(0,0,0);  if (col)  ofs = col->offset;  // mesh offset

			//  num trees  ----------------------------------------------------------------
			int cnt = fTrees * 6000 * pg.dens;
			
			for (int i = 0; i < cnt; ++i)
			{
				#if 0  ///  test shapes, new objects
					int ii = i; // l*cnt+i;
					yaw = (ii * 5) % 360;  // grid
					pos.z = -100 +(ii / 9) * 10;  pos.x = -100 +(ii % 9) * 10;
					Real scl = pg.minScale;
				#else
					yaw = rnd.rand(360.0);
					pos.x = getTerPos();  pos.z = getTerPos();
					Real scl = rnd.rand() * (pg.maxScale - pg.minScale) + pg.minScale;
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


				//  check ter angle  ------------
				float ang = ter0->getAngle(pos.x, pos.z, td.fTriangleSize);
				if (ang > pg.maxTerAng)
					add = false;

				if (!add)  continue;  //

				//  check ter height  ------------
				bool in = ter0->getHeightAt(pos);
				// LogO(fToStr(pos.y));
				if (!in)  add = false;  // outside
				
				if (pos.y < pg.minTerH || pos.y > pg.maxTerH)
					add = false;
				
				if (!add)  continue;  //
				
				//  check if in fluids  ------------
				float fa = sc->GetDepthInFluids(pos);
				if (fa > pg.maxDepth)
					add = false;


				//  check if on road - uses roadDensity.png
				if (imgRoad && r > 0)  //  ----------------
				{
				int mx = (0.5*tws + pos.x)/tws*r,
					my = (0.5*tws + pos.z)/tws*r;

					int c = sc->trRdDist + pg.addRdist;
					int d = c;
					bool bMax = pg.maxRdist < 20; //100 slow-
					if (bMax)
						d = c + pg.maxRdist+1;  // not less than c

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
				if (!add)  continue;  //

				//  check if outside of terrain?
				// if (pos.x < 
				
				if (!add)  continue;  //


				//  **************  add  **************
				Item *item = mgr->createItem( file,
					ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, SCENE_STATIC );
				
				bool alpha = file == "crystal2.mesh" || file == "crystal2.mesh";  // todo: par in xml
				item->setRenderQueueGroup( alpha ? RQG_BatchAlpha : RQG_Road );
				item->setVisibilityFlags( RV_Vegetation );
				app->SetTexWrap(item);

				//  todo: par in xml * scale,aabb?
				auto s4 = file.substr(0,4), s3 = file.substr(0,3);
				bool big = s4 == "palm" || s4 == "tree" || s4 == "jung" || s4 == "pine" ||
					s3 == "gum" || s3 == "fir" || s4 == "crys" || s4 == "shro";
				Real dist =  // how far visible
					big ? i % 3 == 0 ? 1000.f : 600.f : 300.f;
				item->setRenderingDistance( dist * pSet->trees_dist );
				vegetItems.push_back(item);

				SceneNode *node = rootNode->createChildSceneNode( SCENE_STATIC );
				node->attachObject( item );
				node->scale( scl * Vector3::UNIT_SCALE );
				if (big)
					pos.y -= 0.52f;  // par
				// pos.y += std::min( item->getLocalAabb().getMinimum().y, Real(0.0f) ) * -0.1f + lay.down;  //par
				// todo: ter h in few +-xz, get lowest ..
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
				//  ****************************
				
				++pg.cnt;  ++iVegetAll;  // count stats
					
				
				///  🎳 add to bullet world
				#ifndef SR_EDITOR  //  in Game
				int cc = col ? col->shapes.size() : 0;
				//  not found in xml or specified, 1 shape
				bool useTrimesh = !col || cc == 1 && col->shapes[0].type == BLT_Mesh;
				bool noCol = data->objs->colNone[pg.name];

				if (pSet->game.collis_veget && !noCol)
				if (!useTrimesh)
				for (int c=0; c < cc; ++c)  // all shapes
				{
					const BltShape* shp = &col->shapes[c];
					Vector3 pos = pos0;  // restore original place
					Vector3 ofs = shp->offset;
					//  offset shape  pos, rotY, scl
					Vector2 vo;  float yr = Degree(yaw).valueRadians();
					float cyr = cos(yr), syr = sin(yr);
					vo.x = ofs.x * cyr - ofs.y * syr;
					vo.y = ofs.x * syr + ofs.y * cyr;
					pos.x += vo.x * scl;  pos.z += vo.y * scl;

					//  apply pos offset xyz, rotY, mul by scale
					ter0->getHeightAt(pos);
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
					bco->setFriction(shp->friction);  bco->setRestitution(shp->restitution);
					bco->setCollisionFlags(bco->getCollisionFlags() |
						btCollisionObject::CF_STATIC_OBJECT /*| btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);
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
				#endif
			}
		}
		LogO(String("***** Vegetation models count: ") + toStr(iVegetAll) + "  shapes: " + toStr(cntshp));
	}
	LogO(String(":::* Time Trees: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
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
