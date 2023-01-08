#include "pch.h"
#include "Def_Str.h"
#include "GraphicsSystem.h"
#include "GameEntity.h"
#include <OgreItem.h>
using namespace Ogre;


//  Game Entities
//--------------------------------------------------------------------------------------------------------------------------------
struct GameEntityCmp
{
	bool operator () ( const GameEntity *_l, const Matrix4 * RESTRICT_ALIAS _r ) const
	{
		const Transform &transform = _l->mSceneNode->_getTransform();
		return &transform.mDerivedTransform[transform.mIndex] < _r;
	}

	bool operator () ( const Matrix4 * RESTRICT_ALIAS _r, const GameEntity *_l ) const
	{
		const Transform &transform = _l->mSceneNode->_getTransform();
		return _r < &transform.mDerivedTransform[transform.mIndex];
	}

	bool operator () ( const GameEntity *_l, const GameEntity *_r ) const
	{
		const Transform &lTransform = _l->mSceneNode->_getTransform();
		const Transform &rTransform = _r->mSceneNode->_getTransform();
		return &lTransform.mDerivedTransform[lTransform.mIndex] <
				&rTransform.mDerivedTransform[rTransform.mIndex];
	}
};

//-----------------------------------------------------------------------------------
void GraphicsSystem::gameEntityAdded( const GameEntityManager::CreatedGameEntity *cge )
{
	SceneNode *sceneNode = mSceneManager->getRootSceneNode( cge->gameEntity->mType )->
			createChildSceneNode( cge->gameEntity->mType,
									cge->initialTransform.vPos,
									cge->initialTransform.qRot );

	sceneNode->setScale( cge->initialTransform.vScale );

	cge->gameEntity->mSceneNode = sceneNode;

	if( cge->gameEntity->mMoDefinition->moType == MoTypeItem )
	{
		Item *item = mSceneManager->createItem( cge->gameEntity->mMoDefinition->meshName,
												cge->gameEntity->mMoDefinition->resourceGroup,
												cge->gameEntity->mType );

		StringVector materialNames = cge->gameEntity->mMoDefinition->submeshMaterials;
		size_t minMaterials = std::min( materialNames.size(), item->getNumSubItems() );

		for( size_t i=0; i<minMaterials; ++i )
		{
			item->getSubItem( i )->setDatablockOrMaterialName(
				materialNames[i], cge->gameEntity->mMoDefinition->resourceGroup );
		}

		cge->gameEntity->mMovableObject = item;
	}

	sceneNode->attachObject( cge->gameEntity->mMovableObject );

	// Keep them sorted on how Ogre's internal memory manager assigned them memory,
	// to avoid false cache sharing when we update the nodes concurrently.
	const Transform &transform = sceneNode->_getTransform();
	GameEntityVec::iterator itGameEntity = std::lower_bound(
				mGameEntities[cge->gameEntity->mType].begin(),
				mGameEntities[cge->gameEntity->mType].end(),
				&transform.mDerivedTransform[transform.mIndex],
				GameEntityCmp() );
	mGameEntities[cge->gameEntity->mType].insert( itGameEntity, cge->gameEntity );
}

//-----------------------------------------------------------------------------------
void GraphicsSystem::gameEntityRemoved( GameEntity *toRemove )
{
	const Transform &transform = toRemove->mSceneNode->_getTransform();
	GameEntityVec::iterator itGameEntity = std::lower_bound(
				mGameEntities[toRemove->mType].begin(),
				mGameEntities[toRemove->mType].end(),
				&transform.mDerivedTransform[transform.mIndex],
				GameEntityCmp() );

	assert( itGameEntity != mGameEntities[toRemove->mType].end() && *itGameEntity == toRemove );
	mGameEntities[toRemove->mType].erase( itGameEntity );

	toRemove->mSceneNode->getParentSceneNode()->removeAndDestroyChild( toRemove->mSceneNode );
	toRemove->mSceneNode = 0;

	assert( dynamic_cast<Item*>( toRemove->mMovableObject ) );

	mSceneManager->destroyItem( static_cast<Item*>( toRemove->mMovableObject ) );
	toRemove->mMovableObject = 0;
}

//-----------------------------------------------------------------------------------
void GraphicsSystem::updateGameEntities( const GameEntityVec &gameEntities, float weight )
{
	mThreadGameEntityToUpdate   = &gameEntities;
	mThreadWeight               = weight;

	// Note: You could execute a non-blocking scalable task and do something else, you should
	// wait for the task to finish right before calling renderOneFrame or before trying to
	// execute another UserScalableTask (you would have to be careful, but it could work).
	mSceneManager->executeUserScalableTask( this, true );
}


//  💫 update interpolate ..
//-----------------------------------------------------------------------------------
void GraphicsSystem::execute( size_t threadId, size_t numThreads )
{
	size_t currIdx = mCurrentTransformIdx;
	size_t prevIdx = (mCurrentTransformIdx + NUM_GAME_ENTITY_BUFFERS - 1) % NUM_GAME_ENTITY_BUFFERS;

	const size_t objsPerThread = (mThreadGameEntityToUpdate->size() + (numThreads - 1)) / numThreads;
	const size_t toAdvance = std::min( threadId * objsPerThread, mThreadGameEntityToUpdate->size() );

	auto itor = mThreadGameEntityToUpdate->begin() + toAdvance;
	auto end  = mThreadGameEntityToUpdate->begin() +
				std::min( toAdvance + objsPerThread, mThreadGameEntityToUpdate->size() );
	while( itor != end )
	{
		GameEntity *gEnt = *itor;
		Vector3 interpVec = Math::lerp( gEnt->mTransform[prevIdx]->vPos,
										gEnt->mTransform[currIdx]->vPos, mThreadWeight );
		gEnt->mSceneNode->setPosition( interpVec );

		interpVec = Math::lerp( gEnt->mTransform[prevIdx]->vScale,
								gEnt->mTransform[currIdx]->vScale, mThreadWeight );
		gEnt->mSceneNode->setScale( interpVec );

		Quaternion interpQ = Quaternion::nlerp( mThreadWeight,
			gEnt->mTransform[prevIdx]->qRot,
			gEnt->mTransform[currIdx]->qRot, true );
		gEnt->mSceneNode->setOrientation( interpQ );

		++itor;
	}
}

//--------------------------------------------------------------------------------------------------------------------------------
void GraphicsSystem::processIncomingMessage( Mq::MessageId messageId, const void *data )
{
	switch( messageId )
	{
	case Mq::LOGICFRAME_FINISHED:
		{
			uint32 newIdx = *reinterpret_cast<const uint32*>( data );

			if( newIdx != std::numeric_limits<uint32>::max() )
			{
				mAccumTimeSinceLastLogicFrame = 0;
				//Tell the LogicSystem we're no longer using the index previous to the current one.
				this->queueSendMessage( mLogicSystem, Mq::LOGICFRAME_FINISHED,
										(mCurrentTransformIdx + NUM_GAME_ENTITY_BUFFERS - 1) %
										NUM_GAME_ENTITY_BUFFERS );

				assert( (mCurrentTransformIdx + 1) % NUM_GAME_ENTITY_BUFFERS == newIdx &&
						"Graphics is receiving indices out of order!!!" );

				//Get the new index the LogicSystem is telling us to use.
				mCurrentTransformIdx = newIdx;
			}
		}
		break;
	case Mq::GAME_ENTITY_ADDED:
		gameEntityAdded( reinterpret_cast<const GameEntityManager::CreatedGameEntity*>( data ) );
		break;
	case Mq::GAME_ENTITY_REMOVED:
		gameEntityRemoved( *reinterpret_cast<GameEntity * const *>( data ) );
		break;
	case Mq::GAME_ENTITY_SCHEDULED_FOR_REMOVAL_SLOT:
		//Acknowledge/notify back that we're done with this slot.
		this->queueSendMessage( mLogicSystem, Mq::GAME_ENTITY_SCHEDULED_FOR_REMOVAL_SLOT,
								*reinterpret_cast<const uint32*>( data ) );
		break;
	default:
		break;
	}
}
