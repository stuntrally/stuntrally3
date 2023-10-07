#pragma once
#include "OgreCommon.h"
#include <OgreMovableObject.h>
#include <OgreRenderable.h>
// #include <OgreVertexBufferPacked.h>
namespace Ogre {  class VertexBufferPacked;  }


class HudRenderable final
	: public Ogre::MovableObject, public Ogre::Renderable
{
	//  for checks
	Ogre::OperationType mOper = Ogre::OT_TRIANGLE_LIST;
	bool bUV = false, bColors = false, b3D = false;
	Ogre::String sMtr;

	int iVertCount = 0, iVertCur = 0;  // total, current
	int iIndCount = 0;  // indices
	int iFace = 0;  // indices per face/line

public:
	//  🌟 ctor, see cpp for details
	HudRenderable(
		const Ogre::String& material, Ogre::SceneManager* mgr,
		Ogre::OperationType oper, bool hasUV, bool colors,
		Ogre::uint32 vis, Ogre::uint8 rndQue,
		int count, bool is3D = false);
	~HudRenderable() override;

	//  from MovableObject
	const Ogre::String &getMovableType() const override;
	
	//  from Renderable
	const Ogre::LightList &getLights() const override;
	void getRenderOperation( Ogre::v1::RenderOperation &op, bool casterPass ) override;
	void getWorldTransforms( Ogre::Matrix4 *xform ) const override;
	bool getCastsShadows() const override;


	//  💫 Update
	Ogre::VertexBufferPacked *vb = 0;
	void createBuffers(const int count);
	void destroy();

	//  ManualObject syntax, update all verts
	float *RESTRICT_ALIAS vp = 0;
	void begin();
	void position(float x, float y, float z);
	void texUV(float u, float v);
	void color(float r, float g, float b, float a);
	void color(Ogre::ColourValue c);
	void end();
};
