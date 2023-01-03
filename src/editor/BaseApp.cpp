#include "pch.h"
#include "Def_Str.h"
#include "BaseApp.h"
#include "settings.h"
#include "Road.h"

#include <OgreOverlayElement.h>
#include <OgreCamera.h>
// #include <OgreRenderTarget.h>
// #include <OgreWindow.h>
// #include <OgreTextureManager.h>
#include <OgreMeshManager.h>
#include <OgreSceneNode.h>
#include <MyGUI_TextBox.h>
using namespace Ogre;


///  Fps stats
//------------------------------------------------------------------------
void BaseApp::updateStats()
{
	//  camera pos, rot
	if (pSet->camPos)
	{
		const Vector3& pos = /*road ? road->posHit :*/ mCamera->getDerivedPosition();
		Vector3 dir = mCamera->getDirection();  //const Quaternion& rot = mCameras[0]->getDerivedOrientation();
		String s =  TR("#{Obj_Pos}: ")+fToStr(pos.x,1)+" " + fToStr(pos.y,1) + " " + fToStr(pos.z,1)+
				TR(", | #{Obj_Rot}: ")+fToStr(dir.x,2) + " "+fToStr(dir.y,2)+" "+fToStr(dir.z,2);
		txCamPos->setCaption(s);
	}
}
