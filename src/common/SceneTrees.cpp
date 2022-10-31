#include "OgreCommon.h"
#include "OgreVector3.h"
#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "CData.h"
#include "SceneXml.h"
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
	// #include "BtOgreGP.h"
#endif
#include "pathmanager.h"
#include "MersenneTwister.h"

// #include <filesystem>
#include <OgreTimer.h>
#include <Ogre.h>
#include <OgreItem.h>

#include "Terra.h"
using namespace Ogre;


//---------------------------------------------------------------------------------------------------------------
///  Trees  ^ ^ ^ ^
//---------------------------------------------------------------------------------------------------------------

void CScene::DestroyTrees()
{
	// fixme
	// if (grass) {  delete grass->getPageLoader();  delete grass;  grass=0;   }
	LogO("------  # Destroy trees");
	// if (trees) {  delete trees->getPageLoader();  delete trees;  trees=0;   }
	LogO("------  # Destroy trees done");
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
	LogO("# CreateTrees()");
	Ogre::Timer ti;
	cntAll = 0;
	// updGrsTer();
		
	//-------------------------------------- Grass --------------------------------------
	int imgRoadSize = 0;
	/*Image imgRoad;  //;
	try{
		imgRoad.load(String("roadDensity.png"),"General");
	}catch(...)
	{	LogO("Warning: Trees can't load roadDensity !");
	}
	imgRoadSize = imgRoad.getWidth();  // square[]
	
	roadDens.Load(TrkDir()+"objects/roadDensity.png");
	
	UpdGrassDens();  //!
	*/
	
	Real tws = sc->td.fTerWorldSize * 0.5f;
	// TBounds tbnd(-tws, -tws, tws, tws);
	//  pos0 - original  pos - with offset
	Vector3 pos0 = Vector3::ZERO, pos = Vector3::ZERO;  Radian yaw;

	SETTINGS* pSet = app->pSet;
	Real fGrass = pSet->grass * sc->densGrass * 3.0f;  // std::min(pSet->grass, 
	#ifdef SR_EDITOR
	Real fTrees = pSet->gui.trees * sc->densTrees;
	#else
	Real fTrees = pSet->game.trees * sc->densTrees;
	#endif

	#ifdef CAR_PRV
	fTrees = 1.0f;
	fGrass = 1.0f;
	#endif
	
#if 0
	if (fGrass > 0.f)
	{
		grass->addDetailLevel<GrassPage>(sc->grDist * pSet->grass_dist);
		grassLoader->setRenderQueueGroup(RQG_BatchAlpha);

		//  Grass layers
		const SGrassLayer* g0 = &sc->grLayersAll[0];
		for (int i=0; i < sc->ciNumGrLay; ++i)
		{
			SGrassLayer* gr = &sc->grLayersAll[i];
			if (gr->on)
			{
				GrassLayer *l = grassLoader->addLayer(gr->material);
				l->setMinimumSize(gr->minSx, gr->minSy);
				l->setMaximumSize(gr->maxSx, gr->maxSy);
				l->setDensity(gr->dens * fGrass);

				l->setSwayDistribution(g0->swayDistr);
				l->setSwayLength(g0->swayLen);  l->setSwaySpeed(g0->swaySpeed);

				l->setAnimationEnabled(true);  //l->setLightingEnabled(true);
				l->setRenderTechnique(GRASSTECH_CROSSQUADS);  //GRASSTECH_SPRITE-
				l->setFadeTechnique(FADETECH_ALPHA);  //FADETECH_GROW-

				l->setColorMap(gr->colorMap);
				
				l->setDensityMap(grassDensRTex, MapChannel(gr->iChan));
				l->setMapBounds(tbnd);
				gr->grl = l;
			}
		}
		grass->setShadersEnabled(true);
	}
	LogO(String("::: Time Grass: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  ti.reset();
#endif

	//---------------------------------------------- Trees ----------------------------------------------
	if (fTrees > 0.f)
	{
		// if (!pSet->impostors_only)
		// 	trees->addDetailLevel<WindBatchPage>(sc->trDist * pSet->trees_dist, 0);
		// 	trees->addDetailLevel<ImpostorPage>(sc->trDistImp * pSet->trees_dist, 0);
		ResourceGroupManager& resMgr = ResourceGroupManager::getSingleton();
		SceneManager *mgr = app->mSceneMgr;
		SceneNode *rootNode = mgr->getRootSceneNode( SCENE_STATIC );

		tws = sc->td.fTerWorldSize;
		int r = imgRoadSize, cntshp = 0, txy = sc->td.iVertsX*sc->td.iVertsY-1;

		//  set random seed  /// add seed in scene.xml and in editor gui..
		MTRand rnd((MTRand::uint32)1213);
		#define getTerPos()		(rnd.rand()-0.5) * sc->td.fTerWorldSize

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
				#if 0  ///  for new objects - test shapes
					int ii = l*cnt+i;
					yaw = Degree((ii*30)%360);  // grid
					pos.z = -100 +(ii / 12) * 10;  pos.x = -100 +(ii % 12) * 10;
					Real scl = pg.minScale;
				#else
					yaw = Degree(rnd.rand(360.0));
					pos.x = getTerPos();  pos.z = getTerPos();
					Real scl = rnd.rand() * (pg.maxScale - pg.minScale) + pg.minScale;
				#endif
				pos0 = pos;  // store original place
				bool add = true;

				//  offset mesh  pos, rotY, scl
				Vector2 vo;  float yr = -yaw.valueRadians();
				float cyr = cos(yr), syr = sin(yr);
				vo.x = ofs.x * cyr - ofs.y * syr;  // ofs x,y for pos x,z
				vo.y = ofs.x * syr + ofs.y * cyr;
				pos.x += vo.x * scl;  pos.z += vo.y * scl;
				
			#if 0
				//  check if on road - uses roadDensity.png
				if (r > 0)  //  ----------------
				{
				int mx = (pos.x + 0.5*tws)/tws*r,
					my = (pos.z + 0.5*tws)/tws*r;

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
						float cr = imgRoad.getColourAt(
							std::max(0,std::min(r-1, mx+ii)),
							std::max(0,std::min(r-1, my+jj)), 0).r;
						
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
			#endif

				//;  check ter angle  ------------
				float ang = terrain->getAngle(pos.x, pos.z, sc->td.fTriangleSize);
				if (ang > pg.maxTerAng)
					add = false;

				if (!add)  continue;  //

				//  check ter height  ------------
				terrain->getHeightAt(pos);
				if (pos.y < pg.minTerH || pos.y > pg.maxTerH)
					add = false;
				
				if (!add)  continue;  //
				
				//  check if in fluids  ------------
				float fa = sc->GetDepthInFluids(pos);
				if (fa > pg.maxDepth)
					add = false;

				//  check if outside of terrain?
				// if (pos.x < 
				
				if (!add)  continue;  //


				//  **************  add  **************
				Item *item = mgr->createItem( file,
					ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, SCENE_STATIC );
				item->setVisibilityFlags(RV_Vegetation);

				SceneNode *node = rootNode->createChildSceneNode( SCENE_STATIC );
				node->attachObject( item );
				node->scale( scl * Vector3::UNIT_SCALE );
				// pos.y += std::min( item->getLocalAabb().getMinimum().y, Real(0.0f) ) * -0.1f + lay.down;  //par
				node->setPosition( pos );

				Degree a( Math::RangeRandom(0, 360.f) );
				Quaternion q;  q.FromAngleAxis( a, Vector3::UNIT_Y );
				node->setOrientation( q );
				//  ****************************
				
				// node->scale( s, s, s );
				// treeLoader->addTree(ent, pos0, yaw, scl);
				++pg.cnt;  ++cntAll;  // count stats
					
				
				///  add to bullet world
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
					Vector2 vo;  float yr = -yaw.valueRadians();
					float cyr = cos(yr), syr = sin(yr);
					vo.x = ofs.x * cyr - ofs.y * syr;
					vo.y = ofs.x * syr + ofs.y * cyr;
					pos.x += vo.x * scl;  pos.z += vo.y * scl;

					//  apply pos offset xyz, rotY, mul by scale
					terrain->getHeightAt(pos);
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
				else  // use trimesh  . . . . . . . . . . . . 
				{
				#if 0  // fixme add btOgre
					const BltShape* shp = !col ? &data->objs->defPars : &col->shapes[0];
					Vector3 pc(pos0.x, pos.y, pos0.z);
					Quaternion q;  q.FromAngleAxis(yaw, Vector3::UNIT_Y);
					Matrix4 tre;  tre.makeTransform(pc, scl*Vector3::UNIT_SCALE, q);
					BtOgre::StaticMeshToShapeConverter converter(ent, tre);
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
				#endif
				}
				#endif
			}
		}
		
		LogO(String("***** Vegetation objects count: ") + toStr(cntAll) + "  shapes: " + toStr(cntshp));
	}
	//imgRoadSize = 0;
	LogO(String("::: Time Trees: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}
