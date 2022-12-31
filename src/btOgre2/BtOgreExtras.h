/*
* =====================================================================================
*
*       Filename:  BtOgreExtras.h
*
*    Description:  Contains the Ogre Mesh to Bullet Shape converters.
*
*        Version:  1.0
*        Created:  27/12/2008 01:45:56 PM
*
*         Author:  Nikhilesh (nikki)
*
* =====================================================================================
*/
#pragma once
#include <btBulletDynamicsCommon.h>
#include <OgreSceneNode.h>
#include <OgreSimpleRenderable.h>
#include <OgreCamera.h>
#include <OgreHardwareBufferManager.h>
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreRoot.h>
#include <OgreHlms.h>
#include <OgreHlmsUnlit.h>
#include <OgreLogManager.h>


namespace BtOgre
{
	///Type for array of vector "
	using Vector3Array = std::vector<Ogre::Vector3>;

	///Converts from and to Bullet and Ogre stuff. Pretty self-explanatory.
	struct Convert
	{
		///Do not permit to construct a "BtOgre::Convert" object
		Convert() = delete;

		///Ogre -> Bullet Quaternion
		static btQuaternion toBullet(const Ogre::Quaternion& q);

		///Ogre -> Bullet Vector 3D
		static btVector3 toBullet(const Ogre::Vector3& v);
		static btVector3 toBullet2(const Ogre::Vector3& v);

		///Bullet -> Ogre Quaternion
		static Ogre::Quaternion toOgre(const btQuaternion& q);

		///Bullet -> Ogre Vector 3D
		static Ogre::Vector3 toOgre(const btVector3& v);
	};

	///Draw the lines Bullet want's you to draw
	class LineDrawer
	{
		Ogre::String sceneManagerName;
		///How a line is stored
		struct line
		{
			Ogre::Vector3 start;
			Ogre::Vector3 end;
			Ogre::ColourValue vertexColor;
		};

		///Where the created objects will be attached
		Ogre::SceneNode* attachNode;

		///The name of the HLMS datablock to use
		Ogre::String datablockToUse;

		///Array of lines and objects to use
		std::vector<line> lines;

		///Manual object used to display the lines
		Ogre::ManualObject* manualObject;

		///Pointer to the scene manager containing the physics objects
		Ogre::SceneManager* smgr;

		///Vertex index
		size_t index;

	public:
		///Construct the line drawer, need the name of the scene manager and the datablock (material)
		LineDrawer(Ogre::SceneNode* node, Ogre::String datablockId, Ogre::SceneManager* smgrName);

		///Desstroy the line drawer
		~LineDrawer();

		///Clear the manual object AND the line buffer
		void clear();

		///Add a line to the "line buffer", the list of lines that will be shown at next update
		void addLine(const Ogre::Vector3& start, const Ogre::Vector3& end, const Ogre::ColourValue& value);

		///Check if the material actually exist, if it doesn't create it
		void checkForMaterial() const;

		///Update the content of the manual object with the line buffer
		void update();
	};

	///Debug Drawer, permit to visualize and debug the physics
	class DebugDrawer : public btIDebugDraw
	{
	protected:

		///Node where the debug drawer is attached
		Ogre::SceneNode *mNode;

		///Pointer to the bullet dynamics world
		btDynamicsWorld *mWorld;

		///State of the debug draw : define what will be drawn
		int mDebugMode;

		///Name of the datablock
		static constexpr const char* unlitDatablockName{ "DebugLinesGenerated" };

		///ID of the datblock
		const Ogre::IdString unlitDatablockId;

		///To accommodate the diffuse color -> light value "change" of meaning of the color of a fragment in HDR pipelines, multiply all colors by this value
		float unlitDiffuseMultiplier;

		///Has been stepped already this frame
		bool stepped;

		///The name of the scene manager to get
		Ogre::String scene;

		///Pointer to the SceneManager
		Ogre::SceneManager* smgr;

		///The LineDrawer : Object that draw lines.
		LineDrawer drawer;

	private:

		///Initialization code that has to be called by all overload of the constructor
		void init();
	public:

		///Write messages to the log, with a "BtOgre21" tag
		static void logToOgre(const std::string& message);

		///Name of the resource group to be used by the debug drawer when creating materials (datablocks)
		static constexpr const char* BtOgre21ResourceGroup{ "BtOgre21" };

		///Debug drawer for Bullet.
		/// \param node SceneNode (usually the Root node) where the lines will be attached. A static child node will be created
		/// \param world Pointer to the btDynamicsWolrd you're using
		/// \param smgrName Name of the scene manager you are using
		DebugDrawer(Ogre::SceneNode* node, btDynamicsWorld* world, const Ogre::String& smgrName = "MAIN_SMGR");

		///Debug drawer for Bullet
		/// \param node SceneNode (usually the Root node) where the lines will be attached. A static child node will be created
		/// \param world Pointer to the btDynamicsWolrd you're using
		/// \param smgr Pointer to the SceneManager to use
		DebugDrawer(Ogre::SceneNode* node, btDynamicsWorld* world, Ogre::SceneManager* smgr);

		///Default polymorphic destructor
		virtual ~DebugDrawer() = default;

		///Set the value to multiply the texure. >= 1. Usefull only for HDR rendering
		void setUnlitDiffuseMultiplier(float value);

		///For bullet : add a line to the drawer
		void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

		///Dummy. Rendering text is hard :D
		void draw3dText(const btVector3& location, const char* textString) override;

		///Just render the contact point wit a line
		void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;

		///Redirect erros to the Ogre default log
		void reportErrorWarning(const char* warningString) override;

		///Set the debug mode. Acceptable values are combinations of the flags defined by btIDebugDraw::DebugDrawModes
		void setDebugMode(int isOn) override;

		///get the current debug mode
		int getDebugMode() const override;

		///Step the debug drawer
		void step();
	};
}
