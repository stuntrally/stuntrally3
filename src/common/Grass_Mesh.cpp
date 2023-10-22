#include "pch.h"
#include "Grass.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "App.h"
#include "game.h"
#include "CScene.h"
#include "SceneXml.h"
#include "settings.h"
#include "GraphicsSystem.h"
#include "Terra.h"

#include <OgreCommon.h>
#include <OgreImage2.h>
#include <OgreMeshManager.h>
#include <OgreLogManager.h>
#include <OgrePixelFormatGpu.h>

#include <OgreException.h>
#include <OgreItem.h>
#include <OgreSceneNode.h>
#include <OgreMesh2.h>
#include <OgreSubMesh2.h>
#include <OgreSceneManager.h>
#include <OgreMeshManager2.h>
#include <Vao/OgreVaoManager.h>
#include <OgreMesh.h>
#include <OgreTextureBox.h>
#include <OgreHlmsPbsDatablock.h>
using namespace Ogre;
using namespace std;


//  todo: fake, just to shut up warnings
class ManResLd : public ManualResourceLoader
{
	void prepareResource( Resource *resource ) override
	{
	}
	void loadResource( Resource *resource ) override
	{
	}
} gGrassManResLd;


//  Create  Grass
//---------------------------------------------------------
void Grass::Create(App* app1)
{
	app = app1;
	LogO("C--G Create Grass: ");
	Timer ti;
	if (!pSet || !scn || !terrain || !scn->imgRoad)
		return;

	const auto* sc = scn->sc;
	const Real tws = sc->tds[0].fTerWorldSize, hws = tws * 0.5f;  //par-
	const Real mrg = 0.98f;

	const Real fGrass = pSet->g.grass * sc->densGrass * 3.0f;  // std::min(pSet->grass, 
	#ifdef SR_EDITOR
	const Real fTrees = pSet->gui.trees * sc->densTrees;
	#else
	const Real fTrees = pSet->game.trees * sc->densTrees;
	#endif
	
	if (fGrass <= 0.f)  return;
	LogManager::getSingleton().setLogDetail(LoggingLevel::LL_NORMAL);  // skip warnings for no loader

	// sc->grPage;  GrassPage(sc->grDist * pSet->grass_dist);  // global, not or per track ?
	//gds.reserve(100);
	const int r = scn->imgRoadSize;  //-1?
	//  page size  from ter size
	const Real pws = max(100.f, min(500.f, tws / 5.f));
	// const Real pws = 100.f;  // test
	LogO("Grass page size: "+toStr(pws));
	const Real hps = pws / 2.f;
	const Real pws2 = pws * pws;

	int id = 0, all = 0, mtr = 0;
	TextureBox tb;
	if (scn->imgRoad)
		tb = scn->imgRoad->getData(0);

	//  Grass layers
	const SGrassLayer* g0 = &sc->grLayersAll[0];
	for (int i=0; i < sc->ciNumGrLay; ++i)
	{
		const SGrassLayer* gr = &sc->grLayersAll[i];
		if (gr->on)
		{
			const SGrassChannel* ch = &scn->sc->grChan[gr->iChan];
			int na = 0, pg = 0;  // stats
			++mtr;  //-

			//  pages
			for (Real yy = -hws; yy < hws; yy += pws)
			for (Real xx = -hws; xx < hws; xx += pws)
			{
				//  create grass page
				std::vector<Ogre::Vector3> pos, norm;
				std::vector<Ogre::Vector2> tcs;
				std::vector<Ogre::Vector4> clr; //-
				std::vector<Ogre::uint32> idx;
				++id;  ++pg;
				
				//  page y pos
				const Real xp = xx + hps, zp = yy + hps;  // world pos
				const Real yp = terrain->getHeight(xp, zp);

				//  density  par
				const int nn = gr->dens * fGrass * 2.0f * pws2;
				uint32 ix = 0;

				const size_t ps = nn * 8, is = nn * 6;
				pos.reserve(ps);  norm.reserve(ps);  tcs.reserve(ps);
				idx.reserve(is);

				for (int n = 0; n < nn; ++n)
				{
					const Real xl = Math::RangeRandom(-hps, hps);  // local pos
					const Real zl = Math::RangeRandom(-hps, hps);  // -+ half page
					const Real xw = xl + xp, xn = xw / hws;  // w world
					const Real zw = zl + zp, zn = zw / hws;  // n -1..1
					if (xn <= -mrg || zn <= -mrg ||
						xn >=  mrg || zn >=  mrg)
						continue;  // outside ter

					//  todo: grass density RTT, channels in rgba, image read not this..
					//  ter ang
					Real at = terrain->getAngle(xw, zw, 1.f);
					if (at < ch->angMin - ch->angSm/2.f)  // || at > ch->angMax + ch->angSm/2.f)
						continue;  // todo smooth ..
					Real hi = (at - ch->angMax) / ch->angSm;
					if (hi > 1.f || hi > Math::UnitRandom())
						continue;
					//  ter h
					Real h = terrain->getHeight(xw, zw);  // /2 par..
					if (h < ch->hMin - ch->hSm/2.f || h > ch->hMax + ch->hSm)  // linRange-
						continue;

					//  check if on road - uses roadDensity.png
					const int xrd = (0.5 + 0.5 * xn) * r;  // 0..1
					const int yrd = (0.5 + 0.5 * zn) * r;
					const float cr = tb.getColourAt(
						yrd, xrd, 0, Ogre::PFG_RGBA8_UNORM_SRGB ).r;

					if (cr < 0.65f)  //par gui ..
					// if (cr < 0.05f)  //par
						continue;
					// if (cr < Math::UnitRandom())  // todo: smooth..
					// 	continue;


					//  add new  grass x
					Real sx = Math::RangeRandom(gr->minSx, gr->maxSx) * 0.5f;  // sizes
					Real sy = Math::RangeRandom(gr->minSy, gr->maxSy);
					Real ay = Math::RangeRandom(0, Math::PI);  // rot
					++na;
	
					for (int q=0; q < 2; ++q)  // cross x  // todo: more types, blades, lods?
					for (int y=0; y < 2; ++y)  // rect []
					for (int x=0; x < 2; ++x)
					{
						const Real a = ay + q * Math::HALF_PI + x * Math::PI;
						const Real  // rot y
							xr = cosf(a) * sx,
							zr = sinf(a) * sx;
						Vector3 p(  // pos
							xl + xr, y * sy,
							zl + zr );
						
						p.y += terrain->getHeight(p.x + xp, p.z + zp) - yp;

						Vector2 uv(x, 1.f - y);
						Vector3 n(0,1,0);  // todo: tilt^? bad for car lights
						// Vector3 n( xl + xr, y * 0.f, zl + zr );
						// Vector3 n( cosf(a+Math::HALF_PI), y*0.4f, sinf(a+Math::HALF_PI));  //..
						// Vector3 n( cosf(a), y*0.4f, sinf(a));  //..
						// n.normalise();
						//Vector4 c(1,1,1,1);

						pos.push_back(p);  norm.push_back(n);
						tcs.push_back(uv);  //clr.push_back(c);
					}
					//  todo: no idx, generate in geom/verter shader?!
					idx.push_back(ix+1);  idx.push_back(ix+3);  idx.push_back(ix+0);  // |\ |
					idx.push_back(ix+3);  idx.push_back(ix+2);  idx.push_back(ix+0);  // | \|
					ix += 4;
				}

				if (!pos.empty())  // add mesh
				{	GrassData gd;

					CreateMesh(gd, "g"+toStr(id), gr->material, 
						pos, norm, clr, tcs, idx);
					
					//  color
					Hlms *hlms = app->mRoot->getHlmsManager()->getHlms( HLMS_PBS );
					HlmsPbsDatablock *db = static_cast<HlmsPbsDatablock*>(hlms->getDatablock( gr->material ));
					if (db)
						db->setDiffuse(gr->color.GetRGB());
					
					gd.node->setPosition(Vector3(xp, yp, zp));
					gd.it->setRenderingDistance( 100.f * pSet->g.grass_dist );  //par
					gds.push_back(gd);  ++all;
				}
			}	// pages

			LogO(String("grass x ") +iToStr(na,5) +"  pages "+ toStr(pg) +"  mtr "+ gr->material);
			// g0->swayDistr;
			// g0->swayLen;  g0->swaySpeed;
			// FADETECH_ALPHA  //FADETECH_GROW-
			// gr->colorMap
			// l->setDensityMap(grassDensRTex, MapChannel(gr->iChan));
		}
	}

	LogO(String("C::+ Grass = mtr: ") + toStr(mtr));
	LogO(String("C::+ Grass = meshes: ") + toStr(all));
	LogManager::getSingleton().setLogDetail(LoggingLevel::LL_BOREME);
	LogO(String(":::* Time Grass: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  ti.reset();
}


//  Create Mesh
//---------------------------------------------------------

void Grass::CreateMesh( GrassData& sd, Ogre::String sMesh, Ogre::String sMtrName,
	const std::vector<Ogre::Vector3>& pos, const std::vector<Ogre::Vector3>& norm,
	const std::vector<Ogre::Vector4>& clr, const std::vector<Ogre::Vector2>& tcs,
	const std::vector<Ogre::uint32>& idx)
{
	size_t i, si = pos.size();
	if (si == 0)
	{	LogO("Grass CreateMesh 0 verts");
		return;
	}
	if (MeshManager::getSingleton().getByName(sMesh))
		LogO("Mesh exists !!!" + sMesh);

 	Aabb aabox;
 	MeshPtr mesh = MeshManager::getSingleton().createManual(sMesh, "General", &gGrassManResLd);
	SubMesh* subMesh = mesh->createSubMesh();

    //-----------------------------------------------------------------------------------
    bool partialMesh = false;
	Root *root = scn->app->mGraphicsSystem->getRoot();
	RenderSystem *rs = root->getRenderSystem();
	VaoManager *vaoManager = rs->getVaoManager();

	// Vertex declaration
	VertexElement2Vec vertexElements;
	vertexElements.push_back( VertexElement2( VET_FLOAT3, VES_POSITION ) );
	vertexElements.push_back( VertexElement2( VET_FLOAT3, VES_NORMAL ) );
	vertexElements.push_back( VertexElement2( VET_FLOAT2, VES_TEXTURE_COORDINATES) );
	bool hasClr = clr.size() > 0;
	if (hasClr)
		vertexElements.push_back( VertexElement2( VET_FLOAT4, VES_DIFFUSE ) );


	//  vertex data  ------------------------------------------
	uint vertCnt = pos.size();
	uint vertSize = VaoManager::calculateVertexSize( vertexElements );
	float *vertices = reinterpret_cast<float *>(
		OGRE_MALLOC_SIMD( sizeof( float ) * vertSize * vertCnt, MEMCATEGORY_GEOMETRY ) );
	
	uint a = 0;
	if (hasClr)
	for (uint i=0; i < vertCnt; ++i)
	{
		vertices[a++] = pos[i].x;   vertices[a++] = pos[i].y;   vertices[a++] = pos[i].z;  aabox.merge(pos[i]);
		vertices[a++] = norm[i].x;  vertices[a++] = norm[i].y;  vertices[a++] = norm[i].z;
		vertices[a++] = tcs[i].x;   vertices[a++] = tcs[i].y;
		vertices[a++] = clr[i].x;   vertices[a++] = clr[i].y;   vertices[a++] = clr[i].z;   vertices[a++] = clr[i].w;
	}else
	for (uint i=0; i < vertCnt; ++i)
	{
		vertices[a++] = pos[i].x;   vertices[a++] = pos[i].y;   vertices[a++] = pos[i].z;  aabox.merge(pos[i]);
		vertices[a++] = norm[i].x;  vertices[a++] = norm[i].y;  vertices[a++] = norm[i].z;
		vertices[a++] = tcs[i].x;   vertices[a++] = tcs[i].y;
		// vertices[a++] = tcs[i].x;   vertices[a++] = tcs[i].y;  //2nd uv-
	}

	VertexBufferPacked *vertexBuffer = 0;
	try
	{
		vertexBuffer = vaoManager->createVertexBuffer(
			vertexElements, vertCnt, partialMesh ? BT_DEFAULT : BT_IMMUTABLE, &vertices[0], true );
	}
	catch (Exception &e)
	{
		// we passed keepAsShadow = true to createVertexBuffer, thus Ogre will free the pointer
		// if keepAsShadow = false, YOU need to free the pointer
		OGRE_FREE_SIMD( vertices, MEMCATEGORY_GEOMETRY );
		vertexBuffer = 0;
		throw e;
	}


	//  Vao, one vertex buffer source (multi-source not working yet)
	VertexBufferPackedVec vertexBuffers;
	vertexBuffers.push_back( vertexBuffer );


	//  index data  ------------------------------------------
	IndexBufferPacked *indexBuffer = 0;

	uint idxCnt = idx.size();
	// if (idxCnt >= 65530)
	// 	LogO("Grass!  Index out of 16bit! "+toStr(idxCnt));

	uint idxSize = sizeof( uint32 ) * idxCnt;
	uint32 *indices = reinterpret_cast<uint32 *>(
		OGRE_MALLOC_SIMD( idxSize, MEMCATEGORY_GEOMETRY ) );
	
	// for (uint i=0; i < idxCnt; ++i)
	// 	indices[i] = idx[i];
	memcpy( indices, &idx[0],  idxSize);

	try
	{
		indexBuffer = vaoManager->createIndexBuffer(
			IndexBufferPacked::IT_32BIT, idxCnt, BT_IMMUTABLE, indices, true );
	}
	catch( Exception &e )
	{
		// When keepAsShadow = true, the memory will be freed when the index buffer is destroyed.
		// However if for some weird reason there is an exception raised, the memory will
		// not be freed, so it is up to us to do so.
		OGRE_FREE_SIMD( indices, MEMCATEGORY_GEOMETRY );
		indexBuffer = 0;
		throw e;
	}

	VertexArrayObject *vao =
		vaoManager->createVertexArrayObject( vertexBuffers, indexBuffer, OT_TRIANGLE_LIST );

	// Each Vao pushed to the vector refers to an LOD level.
	// todo: all LODs here
	// Must be in sync with mesh->mLodValues & mesh->mNumLods if > 1 level
	subMesh->mVao[VpNormal].push_back( vao );
	subMesh->mVao[VpShadow].push_back( vao );  // same geometry for shadow casting

	//subMesh->arrangeEfficient(false, false, false);  // no?


	//  add mesh to scene
	//---------------------------------------------------------
	subMesh->setMaterialName(sMtrName);

	mesh->_setBounds(aabox, false);  //?
	mesh->_setBoundingSphereRadius((aabox.getMaximum() - aabox.getMinimum()).length() / 2.0);  
	mesh->load();

	//  tangents-
/*	String s1 = sMesh+"v1", s2 = sMesh+"v2";
	v1::MeshPtr m1 = v1::MeshManager::getSingleton().create(s1, "General",
		v1::HardwareBuffer::HBU_STATIC, v1::HardwareBuffer::HBU_STATIC );
	m1->importV2(mesh.get());
	m1->buildTangentVectors();
	mesh = MeshManager::getSingleton().createByImportingV1(s2, "General", m1.get(), false,false,false);
	MeshManager::getSingleton().remove(sMesh);  // not needed
*/	// faster but will be lost with device..
	mesh->arrangeEfficient(true, true, true);

	//  add mesh to scene
	//---------------------------------------------------------
	Item* it = mSceneMgr->createItem(mesh, SCENE_STATIC );  // no tangents
	// Item* it = mSceneMgr->createItem(s2, "General", SCENE_STATIC );
	// *it = mSceneMgr->createItem( mesh, SCENE_STATIC );
	SceneNode* node = mSceneMgr->getRootSceneNode( SCENE_STATIC )->createChildSceneNode( SCENE_STATIC );
	node->attachObject(it);
	it->setCastShadows(false);  //** ok
	it->setVisibilityFlags(RV_VegetGrass);
	it->setRenderQueueGroup(RQG_BatchAlpha);
	scn->app->SetTexWrap(it, false);

	sd.it = it;
	sd.node = node;
	sd.smesh = sMesh;
	// sd.mesh = mesh;  sd.mesh1 = m1;

	//LogO(sMesh + " " + datablock->getName().getReleaseText());  // hash
}


///  Destroy
//---------------------------------------------------------

void Grass::Destroy()  // full
{
	LogO("D--G destroy Grass");
	for (size_t i=0; i < gds.size(); ++i)
	{
		GrassData& gd = gds[i];
		// if (rs.empty)  return;
		try
		{
			// for (int l=0; l < LODs; ++l)
			{
				if (gd.it)
				{
					mSceneMgr->destroySceneNode(gd.node);
					mSceneMgr->destroyItem(gd.it);

					String sMesh = gd.smesh, s1 = sMesh+"v1", s2 = sMesh+"v2";
					MeshManager::getSingleton().remove(sMesh);
					// MeshManager::getSingleton().remove(s2);  // with tangents
					// v1::MeshManager::getSingleton().remove(s1);
				}
			}
		}
		catch (Exception ex)
		{
			// LogO(String("# Error! grass DestroySeg") + ex.what());
		}
	}
	gds.clear();
	// idStr = 0;
	// LogO("D--- road Destroyed");
	// rs.empty = true;
	// rs.lpos.clear();
}
