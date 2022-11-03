/*
 * =====================================================================================
 *
 *       Filename:  BtOgreGP.h
 *
 *    Description:  The part of BtOgre that handles information transfer from Ogre to
 *                  Bullet (like mesh data for making trimeshes).
 *
 *        Version:  1.1
 *        Created:  27/12/2008 03:29:56 AM
 *
 *         Author:  Nikhilesh (nikki); Arthur Brainville (Ybalrid)
 *
 * =====================================================================================
 */

#pragma once

#include <algorithm>

#include <btBulletDynamicsCommon.h>
#include <Ogre.h>
#include <OgreMesh2.h>
#include <OgreSubMesh2.h>
#include <OgreItem.h>
#include <OgreBitwise.h>

#include <Vao/OgreAsyncTicket.h>
#include <Vao/OgreVertexArrayObject.h>
#include <Vao/OgreVertexBufferPacked.h>
#include <Vao/OgreVertexElements.h>

#include "BtOgreExtras.h"
#include "BtOgre.hpp"

#if OGRE_VERSION_MINOR > 3
#define OGRE_VertexArrayObject_ReadRequests VertexArrayObject::ReadRequestsVec
#else
#define OGRE_VertexArrayObject_ReadRequests VertexArrayObject::ReadRequestsArray
#endif

namespace BtOgre
{
	using BoneIndex = std::map<unsigned, Vector3Array*>;
	using BoneKeyIndex = std::pair<unsigned, Vector3Array*>;

	///Type of a vertex buffer is an vector of Vector3
	using VertexBuffer = std::vector<Ogre::Vector3>;

	///Type of an index buffer is an array of unsigned ints
	using IndexBuffer = std::vector<unsigned int>;

	///
	/// Converter from vertex and index buffer to Bullet BtCollisionShape. Load vertex and index buffer from Ogre Item, Etity, Mesh and v1::Mesh
	///
	class VertexIndexToShape
	{
	public:
		VertexIndexToShape(const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);
		virtual ~VertexIndexToShape();

		///Get the object bounding radius
		Ogre::Real getRadius();

		///Get the object bounding size vector
		Ogre::Vector3 getSize();

		///Get the offset of object bounding box center from (0,0,0) point in mesh
		Ogre::Vector3 getCenterOffset();

		///Return a spherical bullet collision shape from this object
		btSphereShape* createSphere();

		///Return a box bullet collision shape from this object
		btBoxShape* createBox();

		///Return a triangular mesh collision shape from this object
		btBvhTriangleMeshShape* createTrimesh();

		///Return a cynlinder collision shape from this object
		btCylinderShape* createCylinder();

		///Return a convex hull  collision shape from this object
		btConvexHullShape* createConvex();

		///Return a capsule shape from this object
		btCapsuleShape* createCapsule();

		///Get the vertex buffer (array of vector 3)
		const Ogre::Vector3* getVertices();

		///Get the vertex count (size of vertex buffer) of the object
		size_t getVertexCount() const;

		///Get the index buffer of the object (array of unsigned ints)
		const unsigned int* getIndices();

		///Get the index count(size of vertex buffer) from this object
		size_t getIndexCount() const;

		///Get the number of triangles
		size_t getTriangleCount() const;

	protected:

		///Append V2 Vertex data to the vertex buffer
		void appendV1VertexData(const Ogre::v1::VertexData *vertex_data);

		///Add animated vertex data
		void addAnimatedVertexData(const Ogre::v1::VertexData *vertex_data,
			const Ogre::v1::VertexData *blended_data,
			const Ogre::v1::Mesh::IndexMap *indexMap);

		///Load the index data using the given type (16 or 32bit) from a v1 hardwareIndexBuffer
		template<typename T> void loadV1IndexBuffer(Ogre::v1::HardwareIndexBufferSharedPtr ibuf, const size_t& offset,
			const size_t& previousSize, const size_t& appendedIndexes)
		{
			auto pointerData = static_cast<const T*>(ibuf->lock(Ogre::v1::HardwareBuffer::HBL_READ_ONLY));
			for (auto i = 0u; i < appendedIndexes; ++i)
			{
				mIndexBuffer[previousSize + i] = static_cast<unsigned>(offset + pointerData[i]);
			}
			ibuf->unlock();
		}

		///Load Ogre V1 index data and populat the header, can take an offset when going through submesh by submesh
		void appendV1IndexData(Ogre::v1::IndexData *data, const size_t offset = 0);

		//V2 Mesh buffer loading inspired by the solution here: http://www.ogre3d.org/forums/viewtopic.php?f=25&p=522494#p522494

		///Go through the submeshes and set the size of the {vertex;index} buffers
		void getV2MeshBufferSize(const Ogre::Mesh* mesh, size_t& previousVertexSize, size_t& previousIndexSize);

		///load the request and sends it to the VAO manager, this will map all tickets regarding that request, you will need to unmap them when you have finished
		static void requestV2VertexBufferFromVao(Ogre::VertexArrayObject* vao, Ogre::OGRE_VertexArrayObject_ReadRequests& requests);

		///Load the vertex buffer data
		void extractV2SubMeshVertexBuffer(size_t& subMeshOffset, Ogre::OGRE_VertexArrayObject_ReadRequests requests, const size_t& prevSize);

		///Load the index buffer data using the given type (16 or 32bit) from a V2 VAO index async ticket
		template<typename T> void loadV2IndexBuffer(Ogre::AsyncTicketPtr asyncTicket, const size_t& offset,
			const size_t& perviousSize, const size_t& appendedIndexes)
		{
			auto pointerData = static_cast<const T*>(asyncTicket->map());
			for (auto i = 0; i < appendedIndexes; ++i)
			{
				mIndexBuffer[perviousSize + i] = static_cast<unsigned>(offset + pointerData[i]);
			}
			asyncTicket->unmap();
		}

		///Load the index buffer data
		void extractV2SubMeshIndexBuffer(const size_t& previousSize, const size_t& offset, const bool& indices32, Ogre::IndexBufferPacked* indexBuffer);

	protected:

		///Vertex buffer of the object being processed
		VertexBuffer	mVertexBuffer;

		///Index buffer fo the object being processed
		IndexBuffer		mIndexBuffer;

		///Bounds of the object as a AABB min/max box
		Ogre::Vector3	mBounds;
		
		///Offset of AABB center from (0,0,0) point in mesh
		Ogre::Vector3	mCenter;

		///Radius of a sphere that cointains the object bouns
		Ogre::Real		mBoundRadius;

		BoneIndex*		mBoneIndex;

		///Transform to apply to every point of the vertex buffer
		Ogre::Matrix4	mTransform;

		///Scale vector eventually extracted from a parent nodeS
		Ogre::Vector3	mScale;
	};

	///Shape converter for static (non-animated) meshes.
	class StaticMeshToShapeConverter : public VertexIndexToShape
	{
	public:
		///Mesh To Shape converter that take an Ogre V1 Renderable
		StaticMeshToShapeConverter(Ogre::Renderable *rend, const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);

		///Creaate a messh converter from am V1 entity object
		StaticMeshToShapeConverter(Ogre::v1::Entity *entity, const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);

		///Create a mesh converter from a V1 mesh object
		StaticMeshToShapeConverter(Ogre::v1::Mesh *mesh, const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);

		///Create a mesh converter from a V2 Item object
		StaticMeshToShapeConverter(Ogre::Item* item, const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);

		///Default constructor; You can add a mesh/entity later
		StaticMeshToShapeConverter();

		///Default polymorphic destructor
		virtual ~StaticMeshToShapeConverter() = default;

		///Load an Ogre v1 entity
		void addEntity(Ogre::v1::Entity *entity, const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);

		///Load an Ogre v1 Mesh
		void addMesh(const Ogre::v1::Mesh *mesh, const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);

		///Load an Ogre v2 Item
		void addItem(Ogre::Item* item, const Ogre::Matrix4& transform = Ogre::Matrix4::IDENTITY);

		///Load an Ogre v2 Mesh
		void addMesh(const Ogre::Mesh* mesh, const Ogre::Matrix4& transform = Ogre::Matrix4::IDENTITY);

	protected:

		///Stored Entity
		Ogre::v1::Entity*		mEntity;

		///Stored Item
		Ogre::Item*				mItem;

		///Node where the entity and the item are stored (to extract a scale vector)
		Ogre::SceneNode*		mNode;
	};

	///For animated meshes.
	class AnimatedMeshToShapeConverter : public VertexIndexToShape
	{
	public:

		AnimatedMeshToShapeConverter(Ogre::v1::Entity *entity, const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);
		AnimatedMeshToShapeConverter();
		virtual ~AnimatedMeshToShapeConverter();

		void addEntity(Ogre::v1::Entity *entity, const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);
		void addMesh(const Ogre::v1::MeshPtr &mesh, const Ogre::Matrix4 &transform);

		btBoxShape* createAlignedBox(unsigned char bone,
			const Ogre::Vector3 &bonePosition,
			const Ogre::Quaternion &boneOrientation);

		btBoxShape* createOrientedBox(unsigned char bone,
			const Ogre::Vector3 &bonePosition,
			const Ogre::Quaternion &boneOrientation);

	protected:

		bool getBoneVertices(unsigned char bone,
			unsigned int &vertex_count,
			Ogre::Vector3* &vertices,
			const Ogre::Vector3 &bonePosition);

		bool getOrientedBox(unsigned char bone,
			const Ogre::Vector3 &bonePosition,
			const Ogre::Quaternion &boneOrientation,
			Ogre::Vector3 &extents,
			Ogre::Vector3 *axis,
			Ogre::Vector3 &center);

		Ogre::v1::Entity*		mEntity;
		Ogre::SceneNode*	mNode;

		Ogre::Vector3       *mTransformedVerticesTemp;
		size_t               mTransformedVerticesTempSize;
	};
}
