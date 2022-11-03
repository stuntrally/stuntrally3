/*
 * =====================================================================================
 *
 *       Filename:  BtOgrePG.h
 *
 *    Description:  The part of BtOgre that handles information transfer from Bullet to
 *                  Ogre (like updating graphics object positions).
 *
 *        Version:  1.0
 *        Created:  27/12/2008 03:40:56 AM
 *
 *         Author:  Nikhilesh (nikki)
 *
 * =====================================================================================
 */

#pragma once

#include <btBulletDynamicsCommon.h>
#include <OgreSceneNode.h>
#include "BtOgreExtras.h"

namespace BtOgre
{
	//A MotionState is Bullet's way of informing you about updates to an object.
	//Pass this MotionState to a btRigidBody to have your SceneNode updated automaticaly.

	///RigidBody state to comunicate back the physics movements to the graphics
	class RigidBodyState : public btMotionState
	{
	protected:

		///Object transform
		btTransform mTransform;
		///Relative transform between the RigidBody and the pivor of the Ogre Mesh
		btTransform mCenterOfMassOffset;

		///Node that this motion state is affecting
		Ogre::SceneNode *mNode;

	public:

		///Create a rigid body state with a specified transform and offset
		RigidBodyState(Ogre::SceneNode* node, const btTransform& transform, const btTransform& offset = btTransform::getIdentity());

		///Create a simple rigid body state
		RigidBodyState(Ogre::SceneNode* node);

		///Get the world transofrm
		/// \param ret : Output parameter where the transform will be written
		void getWorldTransform(btTransform& ret) const override;

		///Set the world transform
		/// \param in : The transform to use. Will change the derivated position in world, not the one relative to the parnet node
		void setWorldTransform(const btTransform& in) override;

		///Set the world transform without updating Ogre world
		void setWorldTransformNoUpdate(const btTransform& in);

		///Set the node used by this rigid body state
		void setNode(Ogre::SceneNode* node);

		///set offset
		void setOffset(const Ogre::Vector3& offset);
		
		///set offset
		void setOffset(const btVector3& offset);
		
		///set offset
		btVector3 getOffset() const;
	};

	//Softbody-Ogre connection goes here!
}
