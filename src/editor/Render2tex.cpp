#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "GuiCom.h"
#include "CScene.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "Road.h"
#include "paths.h"
#include "HudRenderable.h"

#include <OgrePrerequisites.h>
#include <OgreTimer.h>
#include <OgreWindow.h>
// #include <OgreRectangle2D2.h>  //..
#include <OgreImage2.h>
#include <OgreCamera.h>
#include <OgreTextureGpuManager.h>
#include "Terra.h"
#include <OgreSceneNode.h>
#include <OgreCommon.h>
#include <OgreTextureGpu.h>
#include <OgreDepthBuffer.h>
#include <OgreHlmsPbsPrerequisites.h>
#include <OgreHlmsCommon.h>
#include <OgreHlmsUnlit.h>
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorNodeDef.h>
#include <Compositor/OgreCompositorWorkspaceDef.h>
#include <Compositor/OgreCompositorWorkspace.h>
using namespace Ogre;


//  🆕 Create render 2 textures (terrain, road, view)
// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
void App::CreateRnd2Tex()
{
	const char* strWs[RT_ALL] = {
		"Rtt_RoadDens", "Rtt_RoadPreview", "Rtt_Terrain", "Rtt_View3D", "Rtt_PreView3D"};
		
	auto* texMgr = mSceneMgr->getDestinationRenderSystem()->getTextureGpuManager();
	auto* wsMgr = mRoot->getCompositorManager2();

	scn->sc->tds[0].UpdVals();
	Real tws = scn->sc->tds[0].fTerWorldSize;  // world dim  // 1st ter

	for (int i=0; i < RT_ALL; ++i)
	{
		bool full = i == RT_View3D, prv3D = i == RT_PreView3D;
		auto& r = rt[i];
		String si = toStr(i); //, sMtr = "road_mini_"+si;
		{
			auto* scMgr = prv3D ? prvScene.mgr : mSceneMgr;

			//  🖼️ Texture & RTT
			r.tex = texMgr->createTexture( "EdTex" + si,
				GpuPageOutStrategy::SaveToSystemRam,
				TextureFlags::ManualTexture, TextureTypes::Type2D );
			// const int mul = tws > 1200.f ? 2 : 1, div = mul == 1 ? 1 : 2;
			uint32 res =
				i == RT_View3D ? 1024 :  // preview const
				1024;  // todo: // mul * 1024;  // bigger with ter size  // todo: fix previews..
			
			r.tex->setResolution(res, res);
			r.tex->scheduleTransitionTo( GpuResidency::OnStorage );
			r.tex->setNumMipmaps(1);  // none

			r.tex->setPixelFormat(
				// PFG_RGBA8_UNORM );  // dark jpg, good prv
				PFG_RGBA8_UNORM_SRGB );  // ok jpg, white prv, like old SR
			r.tex->scheduleTransitionTo( GpuResidency::Resident );

			auto flags = TextureFlags::RenderToTexture; // | TextureFlags::AllowAutomipmaps;
			r.rtt = texMgr->createTexture( "EdRtt" + si,
				GpuPageOutStrategy::Discard, flags, TextureTypes::Type2D );
			r.rtt->copyParametersFrom( r.tex );
			r.rtt->scheduleTransitionTo( GpuResidency::Resident );
			// if( flags & TextureFlags::RenderToTexture )
			r.rtt->_setDepthBufferDefaults( DepthBuffer::POOL_DEFAULT, false, PFG_UNKNOWN );


			//  🎥 Camera
			if (r.cam)
			{	scMgr->destroyCamera(r.cam);  r.cam = 0;  }
			if( !r.cam && !prv3D)  // prv has own
			{
				r.cam = scMgr->createCamera( "RttCam" + si, true );
				bool btm = i == RT_RoadDens;
				r.cam->setPosition(Vector3(0, btm ? -1500 : 1500, 0));  //par- max height
				r.cam->setOrientation( btm ?
					Quaternion(Degree(180), Vector3(1,0,0)) * Quaternion(0.5,-0.5,0.5,0.5) :  // bottom
					Quaternion(0.5,-0.5,0.5,0.5));  // top view
				r.cam->setNearClipDistance(0.5);
				r.cam->setFarClipDistance(50000);  // r.cam->setUseRenderingDistance(true);
				r.cam->setAspectRatio(1.0);
				
				if (!full)  // ortho proj
				{	r.cam->setProjectionType(PT_ORTHOGRAPHIC);
					r.cam->setOrthoWindow(tws, tws);
					// r.cam->setOrthoWindow(tws / div, tws / div);  // todo: ?
					// LogO(" tws  "+toStr(tws));//+" mul "+toStr(mul)+" div "+toStr(div));
				}
				// r.cam->setFOVy( Degree(90) );
				// r.cam->setFixedYawAxis( false );
				// r.cam->setShadowRenderingDistance( 300 );  // par-
				// r.cam->setCastShadows(false);
				// rvp->setVisibilityMask(visMask[i]);  // in workspace..
				// r.cam->setCastShadows(false);
			}

			//  🪄 Workspace  ----
			CompositorChannelVec chan(1);
			chan[0] = r.rtt;

			const String name( "RttWrk" + si );  // created from code
			if( !wsMgr->hasWorkspaceDefinition( name ) )
			{
				auto* w = wsMgr->addWorkspaceDefinition( name );
				// w->mEnableForwardPlus = false;
				w->connectExternal( 0, strWs[i], 0 );
			}else
				LogO("Workspace already exists: "+name);

		// #define MANUAL_RTT_UPD  // todo: terrain shadowed..?

			//  add Workspace
			LogO(String("--++ WS add:  Ed ")+strWs[i]+", all: "+toStr(wsMgr->getNumWorkspaces()));

			auto* cam = prv3D ? prvScene.cam : r.cam;
			r.ws = wsMgr->addWorkspace( scMgr, chan, cam, name,
		#ifndef MANUAL_RTT_UPD
				true );  //! slower
			// r.ws->setEnabled(false);
		#else
				false );  // todo: manual update
			// r.ws->_beginUpdate(true);
			// r.ws->_update();  // todo: upd when needed only, skip
			// r.ws->_endUpdate(true);
		#endif


			//  🌍 Hud minimap rect 2d  ----
			{
			String sMtr = "ed_Rtt_" + si;
			const auto hlms = dynamic_cast<HlmsUnlit*>(mRoot->getHlmsManager()->getHlms(HLMS_UNLIT));
			auto* db = dynamic_cast<HlmsUnlitDatablock*>(hlms->getDatablock(sMtr));
			db->setTexture(PBSM_DIFFUSE, rt[i].tex);

			// String sMtr = "circle_smooth";
			r.hr = new HudRenderable(sMtr, mSceneMgr,
				OT_TRIANGLE_LIST,
				true, false, RV_Hud, RQG_Hud2, 1);

			/*r.hr->begin();  Real a = 0.3f;
			r.hr->position(a,0, 0);  r.hr->texUV(0,1);
			r.hr->position(1,0, 0);  r.hr->texUV(1,1);
			r.hr->position(a,1, 0);  r.hr->texUV(0,0);
			r.hr->position(1,1, 0);  r.hr->texUV(1,0);
			r.hr->end();*/

			r.nd = mSceneMgr->getRootSceneNode()->createChildSceneNode();
			r.nd->attachObject(r.hr);
			r.nd->setVisible(false);  // pSet->trackmap);

			BarrierResolve(r.tex);
		}	}
	}

	//  backgr cover prv cam meh-
	hrBck = new HudRenderable("ed_Bck", mSceneMgr,
		OT_TRIANGLE_LIST,
		false, false, RV_Hud, RQG_Hud1, 1);

	hrBck->begin();
	hrBck->position(-1,-1, 0);
	hrBck->position( 1,-1, 0);
	hrBck->position(-1, 1, 0);
	hrBck->position( 1, 1, 0);
	hrBck->end();
	
	ndBck = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	ndBck->attachObject(hrBck);
	ndBck->setVisible(true);

	//  🔺 pos tri on minimap  . . . . . . . .
	if (!ndPos)
	{
		//  car pos tris (form all cars on all viewports)
		SceneNode* rt = mSceneMgr->getRootSceneNode();
		// mpos = Create2D("hud/CarPos", 0.2f, true);  // dot size
		
		hrPos = new HudRenderable("hud/CarPos", mSceneMgr,
			OT_TRIANGLE_LIST, true,false, RV_Hud,RQG_Hud3, 1);
			
		ndPos = rt->createChildSceneNode(SCENE_DYNAMIC);
		ndPos->attachObject(hrPos);
	}
	UpdMiniVis();
	UpdMiniSize();  // assert in Debug map twice-
}

void App::AddListenerRnd2Tex()
{
	if (scn->refl.mWsListener)
	for (int i=0; i < RT_ALL; ++i)
		rt[i].ws->addListener(scn->refl.mWsListener);  // prv cam refl y?..
}


//  💫 update
void App::UpdRnd2Tex()
{
	static int ii = 0;
	++ii;
	for (int i=0; i < RT_ALL; ++i)
	{
		auto& r = rt[i];  bool full = i == RT_View3D;
		if (!r.ws)  continue;

		if (full)
		{	//  prv view same as cam
			r.cam->setPosition(mCamera->getPosition());
			r.cam->setOrientation(mCamera->getOrientation());
		}

	#ifdef MANUAL_RTT_UPD
		// if (ii == 200)
		// 	r.ws->setEnabled(0);
		// if (ii > 1000)
		// {
		/*	ii = 0;*/
		r.ws->_beginUpdate(true);
		r.ws->_update();  // todo: upd when needed only, skip
		r.ws->_endUpdate(true);/**/
	#endif
	
		//  this is each frame
		for (uint8 i = 0u; i < r.tex->getNumMipmaps(); ++i)
		{
			r.rtt->copyTo( r.tex, r.tex->getEmptyBox(i), i,
				r.rtt->getEmptyBox(i), i );
		}
		// r.tex->writeContentsToFile(PATHS::TracksUser()+"/rtt.png", 0,0);
		// }
	}
}


//  💥 destroy
void App::DestroyEdHud()
{
	delete hrBck;  hrBck = 0;
	if (ndBck) {  mSceneMgr->destroySceneNode(ndBck);  ndBck = 0;  }

	for (int i=0; i < ED_TerEditMax; ++i)
	{	delete hrTerC[i];
		if (ndTerC[i]) {  mSceneMgr->destroySceneNode(ndTerC[i]);  ndTerC[i] = 0;  }
	}
	delete hrBrush;  delete hrTerGen;
	if (ndBrush) {  mSceneMgr->destroySceneNode(ndBrush);  ndBrush = 0;  }
	if (ndTerGen) {  mSceneMgr->destroySceneNode(ndTerGen);  ndTerGen = 0;  }
}

//  💥 destroy
void App::DestroyRnd2Tex()
{
	auto* texMgr = mSceneMgr->getDestinationRenderSystem()->getTextureGpuManager();
	auto* cmpMgr = mRoot->getCompositorManager2();

	for (int i=0; i < RT_ALL; ++i)
	{
		SRndTrg& r = rt[i];
		if (r.ws)  {  cmpMgr->removeWorkspace( r.ws );  r.ws = 0;  }
		if (r.cam) {  mSceneMgr->destroyCamera( r.cam );  r.cam = 0;  }
		if (r.rtt) {  texMgr->destroyTexture( r.rtt );  r.rtt = 0;  }
		if (r.tex) {  texMgr->destroyTexture( r.tex );  r.tex = 0;  }

		delete r.hr;  r.hr = 0;
		if (r.nd)  {  mSceneMgr->destroySceneNode( r.nd );  r.nd = 0;  }
	}

	delete hrBck;  hrBck = 0;
	if (ndBck)  {  mSceneMgr->destroySceneNode( ndBck );  ndBck = 0;  }

	delete hrPos;  hrPos = 0;
	if (ndPos)  {  mSceneMgr->destroySceneNode( ndPos );  ndPos = 0;  }
}

//  🗜️ minimap resize
void App::UpdMiniSize()
{
	float wx = mWindow->getWidth(), wy = mWindow->getHeight();
	asp = wx / wy;
	Real s = pSet->size_minimap;

	for (int i=0; i < RT_ALL; ++i)
	{
		auto& r = rt[i];  bool full = i == RT_View3D;
		if (!r.hr)  continue;

		r.hr->begin();
		auto s = full ? 2.f : pSet->size_minimap;
		Real x = 1.f - s / asp, y = -1.f + s;
		Real x0 = i == RT_RoadDens ? 1.f : 0.f,
			 x1 = i == RT_RoadDens ? 0.f : 1.f;
		r.hr->position(x,  -1.f, 0.f);  r.hr->texUV(x0, 1.f);
		r.hr->position(1.f,-1.f, 0.f);  r.hr->texUV(x1, 1.f);
		r.hr->position(x,     y, 0.f);  r.hr->texUV(x0, 0.f);
		r.hr->position(1.f,   y, 0.f);  r.hr->texUV(x1, 0.f);
		r.hr->end();
	}

	s = 0.04f;  // par tri pos size
	if (ndPos)  // setPos in UpdateKey
		ndPos->setScale(s, s, 1);
}

void App::UpdMiniVis()
{
	bool full = edMode == ED_PrvCam;
	for (int i=0; i < RT_ALL; ++i)
	{
		bool vis = 
			full ? i == RT_View3D :
			pSet->trackmap && i == pSet->num_mini;
		if (rt[i].nd)  rt[i].nd->setVisible(vis);
	}
	if (ndBck)  ndBck->setVisible(full);
	if (ndPos)  ndPos->setVisible(pSet->trackmap && !full);
}


//  🔺 pos,rot on minimap
//--------------------------------
void App::UpdMiniPos()
{
	if (ndPos)
	{	Real w = scn->sc->tds[0].fTerWorldSize;  // 1st ter
		Real x = (0.5 - mCamera->getPosition().z / w);
		Real y = (0.5 + mCamera->getPosition().x / w);
		Real s = pSet->size_minimap;
		Real xm1 = 1.f - s / asp, ym1 = -1.f + s, xm2 = 1.f, ym2 = -1.f;
		ndPos->setPosition( xm1+(xm2-xm1)*x, ym1+(ym2-ym1)*y, 0.f);
		ndPos->_getFullTransformUpdated();

		float angrot = mCamera->getOrientation().getYaw().valueDegrees();
		float psx = 0.9f * pSet->size_minimap, psy = psx*asp;  // *par len

		const static float d2r = PI_d/180.f;
		static float px[4],py[4];
		for (int i=0; i<4; i++)
		{
			float ia = 135.f + float(i)*90.f;
			float p = -(angrot + ia) * d2r;
			px[i] = psx*cosf(p);
			py[i] =-psy*sinf(p);
		}
		if (hrPos)  // fixme
		{	hrPos->begin();
			hrPos->position(px[0],py[0], 0);  hrPos->texUV(0, 1);
			hrPos->position(px[1],py[1], 0);  hrPos->texUV(1, 1);
			hrPos->position(px[3],py[3], 0);  hrPos->texUV(0, 0);
			hrPos->position(px[2],py[2], 0);  hrPos->texUV(1, 0);
			hrPos->end();
	}	}
}


///  Image from road
///  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
void App::SaveGrassDens()
{
	int user = pSet->allow_save ? pSet->gui.track_user : 1;
	auto path = gcom->pathTrk[user] + pSet->gui.track;

	if (!rt[RT_RoadDens].tex)  return;
	// rt[RT_RoadDens].ws->_update();  // all have to exist
	//  save
	rt[RT_RoadDens].tex->writeContentsToFile(path+"/objects/roadDensity.png", 0, 0);
	rt[RT_RoadPrv ].tex->writeContentsToFile(path+"/preview/road.png", 0, 0);
	rt[RT_Terrain ].tex->writeContentsToFile(path+"/preview/terrain.jpg", 0, 0);
	// todo: add fluids, objects..
	
return;  // todo: smooth roadDens? on gpu/compute

	Ogre::Timer ti;

	/*for (int i=0; i < RT_View; ++i)  //-1 preview camera manual
	{
		if (!rt[i].tex)  return;
		rt[i].tex->update();  // all have to exist
	}*/

	int n = RT_View3D; // RT_RoadDens
	int w = rt[n].tex->getWidth(), h = rt[n].tex->getHeight();
	using Ogre::uint;
	uint *rd = new uint[w*h];   // road render
	uint *gd = new uint[w*h];   // grass dens
	// PixelBox pb_rd(w,h,1, PFG_RGBA8_UNORM, rd);
	// rt[n].tex->copyContentsToMemory(pb_rd, RenderTarget::FB_FRONT);
	TextureBox tb;
	// rt[n].rtt->copyContentsToMemory(tb, 
	// rt[n].tex->save

	const int f = std::max(0, scn->sc->grDensSmooth);
	float sum = 0.f;
	int v,i,j,x,y, a,b,d,m;

	//  gauss kernel for smoothing
	int *mask = new int[(f*2+1)*(f*2+1)];  m = 0;
	if (f==0)
	{	mask[0] = 256.f;  sum = 256.f;  }
	else
	for (j = -f; j <= f; ++j)
	for (i = -f; i <= f; ++i, ++m)
	{
		v = std::max(0.f, (1.f - sqrtf((float)(i*i+j*j)) / float(f)) * 256.f);
		mask[m] = v;  sum += v;
	}
	sum = 2.f / sum;  //par normally would be 1.f - but road needs to stay black and be smooth outside
	//  change smooth to distance from road with fade ?..
		
	///  road - rotate, smooth  -----------
	for (y = f; y < h-f; ++y) {  a = y*w +f;
	for (x = f; x < w-f; ++x, ++a)
	{	b = x*w + (h-y);  // rot 90 ccw  b=a
		
		//  sum kernel
		v = 0;  m = 0;
		for (j = -f; j <= f; ++j) {  d = b -f + j*w;
		for (i = -f; i <= f; ++i, ++d, ++m)
			v += ((rd[d] >> 16) & 0xFF) * mask[m] / 256;  }

		v = std::max(0, (int)(255.f * (1.f - v * sum) ));  // avg, inv, clamp
		
		gd[a] = 0xFF000000 + v;  // write
	}	}

	v = 0xFF0000FF;  //  frame f []  todo: get from rd[b] not clear..
	for (y = 0;  y <= f; ++y)	for (x=0; x < w; ++x)	gd[y*w+x] = v;  // - up
	for (y=h-f-1; y < h; ++y)	for (x=0; x < w; ++x)	gd[y*w+x] = v;  // - down
	for (x = 0;  x <= f; ++x)	for (y=0; y < h; ++y)	gd[y*w+x] = v;  // | left
	for (x=w-f-1; x < w; ++x)	for (y=0; y < h; ++y)	gd[y*w+x] = v;  // | right

	Image2 im;  // for trees, before grass angle and height
	im.loadDynamicImage((uchar*)gd, w,h,1, TextureTypes::Type2D, PFG_RGBA8_UNORM, true);
	im.save(gcom->TrkDir()+"objects/roadDensity.png", 0, 1);

	LogO(String(":::* Time road dens: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  ti.reset();

	delete[] rd;  delete[] gd;  delete[] mask;

	LogO(String(":::* Time save prv : ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}


///  pre and post  rnd to tex
//-----------------------------------------------------------------------------------------------------------
/*void App::preRenderTargetUpdate(const RenderTargetEvent &evt)
{
	const String& s = evt.source->getName();
	int num = atoi(s.substr(s.length()-1, s.length()-1).c_str());
	
	if (num == RT_View)  // full
	{
		rt[RT_View].cam->setPosition(mCamera->getPosition());
		rt[RT_View].cam->setDirection(mCamera->getDirection());
	}
	else for (auto r : scn->roads)
		if (!r->IsRiver())
			r->SetForRnd(num == RT_Road ? "render_clr" : "render_grass");
}

void App::postRenderTargetUpdate(const RenderTargetEvent &evt)
{	
	const String& s = evt.source->getName();
	int num = atoi(s.substr(s.length()-1, s.length()-1).c_str());

	if (num != RT_View)
	for (auto r : scn->roads)
	{	if (!r->IsRiver())  r->UnsetForRnd();
		r->UpdLodVis(pSet->road_dist);
	}
	//mCamera->setFarClipDistance(pSet->view_distance);
	//mCamera->setNearClipDistance(0.1f);
}
*/

///  save water depth map  .. drop this
//-----------------------------------------------------------------------------------------------------------
void App::SaveWaterDepth()
{
	return;  //!
	if (scn->sc->fluids.empty())
	{
		gui->Delete(gcom->TrkDir()+"objects/waterDepth.png");  // no tex if no fluids
		return;
	}
	Ogre::Timer ti;

	//  2048 for bigger terrains ?
	int w = 1024, h = w;  float fh = h-1, fw = w-1;
	using Ogre::uint;
	uint *wd = new uint[w*h];   // water depth
	int x,y,a,i,ia,id;
	float fa,fd;
	
	///  write to img  -----------
	//  get ter height to fluid height difference for below
	for (y = 0; y < h; ++y) {  a = y*w;
	for (x = 0; x < w; ++x, ++a)
	{
		//  pos 0..1
		float fx = float(y)/fh, fz = float(x)/fw;
		//  pos on ter  -terSize..terSize
		float w = scn->sc->tds[0].fTerWorldSize;  // 1st ter
		float wx = (fx-0.5f) * w, wz = -(fz-0.5f) * w;

		fa = 0.f;  // fluid y pos
		for (i=0; i < scn->sc->fluids.size(); ++i)
		{
			const FluidBox& fb = scn->sc->fluids[i];
			const float sizex = fb.size.x*0.5f, sizez = fb.size.z*0.5f;
			//  check rect 2d - no rot !  todo: make 2nd type circle..
			if (wx > fb.pos.x - sizex && wx < fb.pos.x + sizex &&
				wz > fb.pos.z - sizez && wz < fb.pos.z + sizez)
			{
				// float f = fb.pos.y - scn->terrain->getHeight(fx,fz);
				// if (f > fa)  fa = f;
			}
		}		//par
		fd = fa * 0.4f * 255.f;  // depth far  full at 2.5 m
		fa = fa * 8.f * 255.f;  // alpha near  full at 1/8 m

		ia = std::max(0, std::min(255, (int)fa ));  // clamp
		id = std::max(0, std::min(255, (int)fd ));
		
		// wd[a] = 0xFF000000 + /*0x01 */ ia + 0x0100 * id;  // write
	}	}

	Image2 im;  // save img
	im.loadDynamicImage((uchar*)wd, w,h,1, TextureTypes::Type2D, PFG_RGBA8_UNORM, true);
	im.save(gcom->TrkDir()+"objects/waterDepth.png", 0, 1);
	delete[] wd;

	/*try {
	TextureGpu* tex = TextureManager::getSingleton().getByName("waterDepth.png");
	if (tex)
		tex->reload();
	else  // 1st fluid after start, refresh matdef ?..
		TextureManager::getSingleton().load("waterDepth.png", rgDef);
	} catch(...) {  }*/

	LogO(String(":::* Time WaterDepth: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}
