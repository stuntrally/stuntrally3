#include "BtOgreExtras.h"
#include "RenderConst.h"
#include <utility>

#include <OgreManualObject2.h>

using namespace Ogre;
using namespace BtOgre;

btQuaternion Convert::toBullet(const Quaternion& q)
{
	return { q.x, q.y, q.z, q.w };
}

btVector3 Convert::toBullet(const Vector3& v)
{
	return { v.x, v.z, v.y };  ///!
}

btVector3 Convert::toBullet2(const Vector3& v)
{
	return { v.x, -v.z, v.y };  ///!
}

Quaternion Convert::toOgre(const btQuaternion& q)
{
	return { (Real)q.w(), (Real)q.x(), (Real)q.y(), (Real)q.z() };
}

Vector3 Convert::toOgre(const btVector3& v)
{
	return { (Real)v.x(), (Real)v.z(), -(Real)v.y() };  ///!
}

LineDrawer::LineDrawer(SceneNode* node, String datablockId, SceneManager* smgr) :
	attachNode(node),
	datablockToUse(std::move(datablockId)),
	manualObject(nullptr),
	smgr(smgr),
	index(0)
{
}

LineDrawer::~LineDrawer()
{
	clear();
	if (manualObject)
		smgr->destroyManualObject(manualObject);
}

void LineDrawer::clear()
{
	if (manualObject) manualObject->clear();
	lines.clear();
}

void LineDrawer::addLine(const Vector3& start, const Vector3& end, const ColourValue& value)
{
	lines.push_back({ start, end, value });
}

void LineDrawer::checkForMaterial() const
{
	const auto hlmsUnlit = dynamic_cast<HlmsUnlit*>(Root::getSingleton().getHlmsManager()->getHlms(HLMS_UNLIT));
	if (!hlmsUnlit)
		throw std::runtime_error("HlmsUnlit not loaded. The debug drawer needs HlmsUnlit to draw unlit shapes");

	const auto datablock = hlmsUnlit->getDatablock(datablockToUse);

	if (datablock) return;
	DebugDrawer::logToOgre("BtOgre's datablock not found, creating...");
	const auto createdDatablock = hlmsUnlit->createDatablock(datablockToUse, datablockToUse, {}, {}, {}, true, BLANKSTRING, DebugDrawer::BtOgre21ResourceGroup);

	if (!createdDatablock) throw std::runtime_error(std::string("BtOgre Line Drawer failed to create HLMS Unlit datablock ") + datablockToUse);
}

void LineDrawer::update()
{
	if (!manualObject)
	{
		DebugDrawer::logToOgre("Create manual object");

		manualObject = smgr->createManualObject(SCENE_STATIC);
		DebugDrawer::logToOgre("Set no shadows");
		manualObject->setCastShadows(false);
		manualObject->setVisibilityFlags(RQG_Hud1);  //** ? still in refl
		DebugDrawer::logToOgre("Attach object to node");
		attachNode->attachObject(manualObject);
		DebugDrawer::logToOgre("done creating object");
	}

	checkForMaterial();
	manualObject->begin(datablockToUse, OT_LINE_LIST);

	index = 0;
	for (const auto& l : lines)
	{
		manualObject->position(l.start);
		manualObject->colour(l.vertexColor);
		manualObject->index(index++);

		manualObject->position(l.end);
		manualObject->colour(l.vertexColor);
		manualObject->index(index++);
	}

	manualObject->end();
}

void DebugDrawer::logToOgre(const std::string& message)
{
	Ogre::LogManager::getSingleton().logMessage("BtOgre21Log : " + message);
}

DebugDrawer::DebugDrawer(SceneNode* node, btDynamicsWorld* world, const String& smgrName) :
	mNode(node->createChildSceneNode(SCENE_STATIC)),
	mWorld(world),
	mDebugMode(true),
	unlitDatablockId(unlitDatablockName),
	unlitDiffuseMultiplier(1),
	stepped(false),
	scene(smgrName),
	smgr(Ogre::Root::getSingleton().getSceneManager(smgrName)),
	drawer(mNode, unlitDatablockName, smgr)
{
	init();
}

DebugDrawer::DebugDrawer(SceneNode* node, btDynamicsWorld* world, SceneManager* smgr) :
	mNode(node->createChildSceneNode(SCENE_STATIC)),
	mWorld(world),
	mDebugMode(true),
	unlitDatablockId(unlitDatablockName),
	unlitDiffuseMultiplier(1),
	stepped(false),
	scene("nonamegiven"),
	smgr(smgr),
	drawer(mNode, unlitDatablockName, smgr)
{
	init();
}

void DebugDrawer::init()
{
	if (!ResourceGroupManager::getSingleton().resourceGroupExists(BtOgre21ResourceGroup))
		ResourceGroupManager::getSingleton().createResourceGroup(BtOgre21ResourceGroup);
}

void DebugDrawer::setUnlitDiffuseMultiplier(float value)
{
	if (value >= 1) unlitDiffuseMultiplier = value;
}

void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	if (stepped)
	{
		drawer.clear();
		stepped = false;
	}

	const auto ogreFrom = Convert::toOgre(from);
	const auto ogreTo = Convert::toOgre(to);

	ColourValue ogreColor{ (Real)color.x(), (Real)color.y(), (Real)color.z(), 1.0f };
	ogreColor *= unlitDiffuseMultiplier;

	drawer.addLine(ogreFrom, ogreTo, ogreColor);
}

void DebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
	//TODO maybe, actually, you know, render text somewhere?
}

void DebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
	(void)lifeTime;
	drawLine(PointOnB, PointOnB + normalOnB * distance * 20, color);
}

void DebugDrawer::reportErrorWarning(const char* warningString)
{
	logToOgre(warningString);
}

void DebugDrawer::setDebugMode(int mode)
{
	mDebugMode = mode;

	if (!mDebugMode)
		drawer.clear();
}

int DebugDrawer::getDebugMode() const
{
	return mDebugMode;
}

void DebugDrawer::step()
{
	if (mDebugMode)
	{
		mWorld->debugDrawWorld();
		drawer.update();
	}
	else
	{
		drawer.clear();
	}
	stepped = true;
}
