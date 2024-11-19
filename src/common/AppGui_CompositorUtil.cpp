#include "pch.h"
#include "Def_Str.h"
#include "settings.h"
#include "settings_com.h"
#include "AppGui.h"
#include "Cam.h"

#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreVector3.h>
#include <OgreWindow.h>
#include <OgreSceneManager.h>
#include <OgreTextureGpuManager.h>
#include <OgreHlmsManager.h>
#include <OgreDepthBuffer.h>
#include <OgrePixelFormatGpu.h>
#include <OgreRenderPassDescriptor.h>

#include <Compositor/Pass/OgreCompositorPassDef.h>
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorNodeDef.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/OgreCompositorWorkspaceDef.h>
#include <Compositor/OgreCompositorWorkspaceListener.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassScene.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h>
#include <Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h>
#include <Compositor/Pass/PassDepthCopy/OgreCompositorPassDepthCopy.h>
#include <Compositor/Pass/PassDepthCopy/OgreCompositorPassDepthCopyDef.h>
#include <Compositor/Pass/PassIblSpecular/OgreCompositorPassIblSpecularDef.h>
using namespace Ogre;


//  util methods
//-----------------------------------------------------------------------------------------

void AppGui::AddShadows(CompositorPassSceneDef* ps)
{
	switch (pSet->g.shadow_type)  // shadows
	{
	case Sh_None:  break;  // none
	case Sh_Depth: ps->mShadowNode = csShadow;  break;
	case Sh_Soft:  ps->mShadowNode = chooseEsmShadowNode();  break;
	}
}

String AppGui::getSplitMtr(int splits)
{
	String mtr;
#ifndef SR_EDITOR
	bool hor = !pSet->split_vertically;
	const String ss = "SplitScreen_";
	switch (splits)
	{
	case 2:  mtr = ss + (hor ? "2h" : "2v");  break;
	case 3:  mtr = ss + (hor ? "3h" : "3v");  break;
	case 4:  mtr = ss + "4";  break;
	case 5:  mtr = ss + (hor ? "5h" : "5v");  break;
	case 6:  mtr = ss + "6";  break;
	}
#endif
	return mtr;
}

//  add NodeDef
CompositorNodeDef* AppGui::AddNode(String name)
{
	auto* mgr = mRoot->getCompositorManager2();
	auto* nd = mgr->addNodeDefinition(name);
	vNodes.push_back(nd);
	LogO("C+-* Add Node " + name);
	return nd;
};
//  add Workspace
CompositorWorkspaceDef* AppGui::AddWork(String name)
{
	auto* mgr = mRoot->getCompositorManager2();
	auto* wd = mgr->addWorkspaceDefinition(name);
	vWorkDefs.push_back(wd);
	LogO("C+-# Add Work " + name);
	return wd;
};

//  add pass  Quad
CompositorPassQuadDef* AppGui::AddQuad(CompositorTargetDef* td)
{
	return static_cast<CompositorPassQuadDef*>(td->addPass(PASS_QUAD));
}

//  add pass  Scene
CompositorPassSceneDef* AppGui::AddScene(CompositorTargetDef* td)
{
	return static_cast<CompositorPassSceneDef*>(td->addPass(PASS_SCENE));
}


//  add Rtv
RenderTargetViewDef* AppGui::AddRtv(CompositorNodeDef* nd,
	String name, String color, String depth, String color2, String color3)
{
	auto* rtv = nd->addRenderTextureView( name );
	rtv->colourAttachments.clear();
	RenderTargetViewEntry at;
	if (!color.empty())
	{	at.textureName = color;
		rtv->colourAttachments.push_back(at);
	}
	if (!color2.empty())
	{	at.textureName = color2;
		rtv->colourAttachments.push_back(at);
	}
	if (!color3.empty())
	{	at.textureName = color3;
		rtv->colourAttachments.push_back(at);
	}
	if (!depth.empty())
		rtv->depthAttachment.textureName = depth;
	// rtv->depthBufferId = 0;  // ignored if ^ set
	return rtv;
}


//  🖼️ create  Texture & RTT
//  final 1 player's view, for SplitScreen combine
//-----------------------------------------------------------------------------------------
TextureGpu* AppGui::AddSplitRTT(String id, float width, float height)
{
	auto* texMgr = mSceneMgr->getDestinationRenderSystem()->getTextureGpuManager();
	//  Tex
	auto* tex = texMgr->createTexture( "GTex" + id,
		GpuPageOutStrategy::SaveToSystemRam,
		TextureFlags::ManualTexture, TextureTypes::Type2D );
	
	tex->setResolution(  // dims
		mWindow->getWidth() * width, mWindow->getHeight() * height);
	tex->scheduleTransitionTo( GpuResidency::OnStorage );
	tex->setNumMipmaps(1);  // none

	tex->setPixelFormat( PFG_RGBA8_UNORM_SRGB );
	tex->scheduleTransitionTo( GpuResidency::Resident );
	vTex.push_back(tex);

	//  RTT
	auto flags = TextureFlags::RenderToTexture;
	auto* rtt = texMgr->createTexture( "GRtt" + id,
		GpuPageOutStrategy::Discard, flags, TextureTypes::Type2D );
	rtt->copyParametersFrom( tex );
	rtt->scheduleTransitionTo( GpuResidency::Resident );
	rtt->_setDepthBufferDefaults( DepthBuffer::POOL_DEFAULT, false, PFG_D32_FLOAT );  //?
	vRtt.push_back(rtt);
	return rtt;
}


//  💥 destroy Compositor
//-----------------------------------------------------------------------------------------
void AppGui::DestroyCompositors()
{
	LogO("DD## Destroy Compositor  "+getWsInfo());
	auto* mgr = mRoot->getCompositorManager2();
	auto* texMgr = mSceneMgr->getDestinationRenderSystem()->getTextureGpuManager();

	for (auto* ws : vWorkspaces)
	{
		mgr->removeWorkspace( ws );
	}
	vWorkspaces.clear();
			
	if (mgr->hasShadowNodeDefinition(csShadow))
		mgr->removeShadowNodeDefinition(csShadow);

	for (auto* rt : vRtt)
		texMgr->destroyTexture( rt );
	vRtt.clear();
	for (auto* tx : vTex)
		texMgr->destroyTexture( tx );
	vTex.clear();

	for (auto* wd : vWorkDefs)
	{
		mgr->removeWorkspaceDefinition( wd->getName() );
	}
	vWorkDefs.clear();

	for (auto* nd : vNodes)
	{
		mgr->removeNodeDefinition( nd->getName() );
	}
	vNodes.clear();
	// mgr->removeAllWorkspaces();  // not necessary..? drops all from .compositor
	// mgr->removeAllWorkspaceDefinitions();  // todo: cube, planar, all from code?
	// mgr->removeAllNodeDefinitions();
	// mgr->removeAllShadowNodeDefinitions();
	// mgr->removeAllCameras();  //-
}


//  💥🎥 Destroy Camera
//--------------------------------------------------------------------------------
void AppGui::DestroyCameras()
{
	LogO("DD## destroy Cameras-");
	/*for (auto cam : mCams)  // todo ?
	{
		if (cam.nd)  mSceneMgr->destroySceneNode(cam.nd);  cam.nd = 0;
		if (cam.cam)  mSceneMgr->destroyCamera(cam.cam);
	}*/
	// mSceneMgr->destroyAllCameras();
	mCams.clear();  // for game, not all
}

Cam* AppGui::findCam(String name)
{
	for (Cam& c : mCamsAll)
		if (c.name == name)
			return &c;
	return 0;
}

//  🆕🎥 Create Camera
//--------------------------------------------------------------------------------
Cam* AppGui::CreateCamera(String name,
	SceneNode* nd, Vector3 pos, Vector3 lookAt)
{
	Cam* fc = findCam(name);
	if (fc)
	{
		LogO("---: got cam: " + name);
		mCams.push_back(*fc);
		return fc;
	}
//  #define USEnodes  //- crashes in ogre upd frustum later
#ifdef USEnodes
	if (!nd)
	{	nd = mSceneMgr->getRootSceneNode(SCENE_DYNAMIC)->createChildSceneNode(SCENE_DYNAMIC);
		nd->setName( name + "CamNd" );
		LogO("c--: new cam node: " + name + "CamNd" );
		nd->setPosition( 0, 50, 100 );  // whatever, upd later
	}
#endif
	Camera* cam = mSceneMgr->createCamera( name );
	LogO("c--: new cam: " + name);

	cam->setPosition( pos );
	cam->lookAt( lookAt );
	cam->setNearClipDistance( 0.1f );
	cam->setFarClipDistance( pSet->g.view_distance );
	cam->setAutoAspectRatio( true );
	cam->setLodBias( pSet->g.lod_bias );

#ifdef USEnodes
	cam->detachFromParent();
	nd->attachObject( cam );
#endif

	Cam c;
	c.cam = cam;
#ifdef USEnodes
	c.nd = nd;
#endif
	c.name = name;
	
	mCamsAll.push_back(c);
	mCams.push_back(c);
	return &mCams.back();
}


//---------------------------------------------------------------------------------
#if 0
	//  VR-  Use one node to control both cameras
	mCamerasNode = mSceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )
						->createChildSceneNode( Ogre::SCENE_DYNAMIC );
	mCamerasNode->setName( "Cameras Node" );

	mCamerasNode->setPosition( 0, 5, 15 );

	mEyeCameras[0] = mSceneManager->createCamera( "Left Eye" );
	mEyeCameras[1] = mSceneManager->createCamera( "Right Eye" );

	const Ogre::Real eyeDistance = 0.5f;
	const Ogre::Real eyeFocusDistance = 0.45f;

	for( int i = 0; i < 2; ++i )
	{
		const Ogre::Vector3 camPos( eyeDistance * ( Ogre::Real( i ) * 2 - 1 ), 0, 0 );
		mEyeCameras[i]->setPosition( camPos );

		Ogre::Vector3 lookAt( eyeFocusDistance * ( Ogre::Real( i ) * 2 - 1 ), -5, -15 );
		// Ogre::Vector3 lookAt( 0, 0, 0 );

		// Look back along -Z
		mEyeCameras[i]->lookAt( lookAt );
		mEyeCameras[i]->setNearClipDistance( 0.2f );
		mEyeCameras[i]->setFarClipDistance( 1000.0f );
		mEyeCameras[i]->setAutoAspectRatio( true );

		// By default cameras are attached to the Root Scene Node.
		mEyeCameras[i]->detachFromParent();
		mCamerasNode->attachObject( mEyeCameras[i] );
	}

	mCamera = mEyeCameras[0];
#endif


//  log util  ----
String AppGui::getWsInfo()
{
	auto* mgr = mRoot->getCompositorManager2();
	return " worksp: "+ toStr(vWorkspaces.size())+" ("+toStr(mgr->getNumWorkspaces())
		+ ") workdef: "+ toStr(vWorkDefs.size())
		+ " node: "+ toStr(vNodes.size())
		+ " rtt: "+ toStr(vRtt.size());
}
