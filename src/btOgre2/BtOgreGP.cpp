/*
 * =============================================================================================
 *
 *       Filename:  BtOgreGP.cpp
 *
 *    Description:  BtOgre "Graphics to Physics" implementation.
 *
 *        Version:  1.1
 *        Created:  27/12/2008 01:47:56 PM
 *
 *         Author:  Nikhilesh (nikki), Arthur Brainville (Ybalrid)
 *
 * =============================================================================================
 */

#include "BtOgrePG.h"
#include "BtOgreGP.h"
#include "BtOgreExtras.h"

#include <Vao/OgreIndexBufferPacked.h>

using namespace Ogre;
using namespace BtOgre;

/*
 * =============================================================================================
 * BtOgre::VertexIndexToShape
 * =============================================================================================
 */

inline void log(const std::string& message)
{
	LogManager::getSingleton().logMessage("BtOgreLog : " + message);
}

void VertexIndexToShape::appendV1VertexData(const v1::VertexData *vertex_data)
{
	if (!vertex_data) return;

	const auto previousSize = mVertexBuffer.size();

	//Resize to fit new data
	mVertexBuffer.resize(previousSize + vertex_data->vertexCount);

	// Get the positional buffer element
	const auto posElem = vertex_data->vertexDeclaration->findElementBySemantic(VES_POSITION);
	const auto vbuf = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());
	const auto vertexSize = static_cast<unsigned int>(vbuf->getVertexSize());

	//Get read only access to the row buffer
	const auto vertex = static_cast<unsigned char*>(vbuf->lock(v1::HardwareBuffer::HBL_READ_ONLY));
	Real* rawVertex{ nullptr }; // this pointer will be used a a buffer to write the [float, float, float] array of the vertex

	//Write data to the vertex buffer
	const auto vertexCount = static_cast<unsigned int>(vertex_data->vertexCount);
	for (auto j = size_t{ 0U }; j < vertexCount; ++j)
	{
		//Get pointer to the start of this vertex
		posElem->baseVertexPointerToElement(vertex + j * vertexSize, &rawVertex);
		mVertexBuffer[previousSize + j] = mTransform * Vector3{ rawVertex };
	}

	//Release vertex buffer opened in read only
	vbuf->unlock();
}

void VertexIndexToShape::addAnimatedVertexData(const v1::VertexData *vertex_data,
	const v1::VertexData *blend_data,
	const v1::Mesh::IndexMap *indexMap)
{
	// Get the bone index element
	assert(vertex_data);

	const auto data = blend_data;

	// Get current size;
	const auto prev_size = mVertexBuffer.size();

	// Get the positional buffer element
	appendV1VertexData(data);

	const auto bneElem = vertex_data->vertexDeclaration->findElementBySemantic(VES_BLEND_INDICES);
	assert(bneElem);

	const auto vbuf = vertex_data->vertexBufferBinding->getBuffer(bneElem->getSource());
	const auto vSize = static_cast<unsigned int>(vbuf->getVertexSize());
	const auto vertex = static_cast<unsigned char*>(vbuf->lock(v1::HardwareBuffer::HBL_READ_ONLY));

	unsigned char* pBone;

	if (!mBoneIndex)
		mBoneIndex = new BoneIndex();
	BoneIndex::iterator i;

	///Todo : get rid of that
	auto curVertices = &mVertexBuffer.data()[prev_size];

	const auto vertexCount = static_cast<unsigned int>(vertex_data->vertexCount);
	for (auto j = size_t{ 0U }; j < vertexCount; ++j)
	{
		bneElem->baseVertexPointerToElement(vertex + j * vSize, &pBone);

		const auto currBone = static_cast<unsigned char>(indexMap ? (*indexMap)[*pBone] : *pBone);
		i = mBoneIndex->find(currBone);

		Vector3Array* l = nullptr;

		if (i == mBoneIndex->end())
		{
			l = new Vector3Array;
			mBoneIndex->insert(BoneKeyIndex(currBone, l));
		}
		else
		{
			l = i->second;
		}

		l->push_back(*curVertices);

		curVertices++;
	}
	vbuf->unlock();
}

void VertexIndexToShape::appendV1IndexData(v1::IndexData *data, const size_t offset)
{
	const auto appendedIndexes = data->indexCount;
	const auto previousSize = mIndexBuffer.size();

	mIndexBuffer.resize(previousSize + appendedIndexes);

	auto ibuf = data->indexBuffer;

	//Test if we need to read 16 or 32 bit indexes
	if (ibuf->getType() == v1::HardwareIndexBuffer::IT_32BIT)
		loadV1IndexBuffer<uint32_t>(ibuf, offset, previousSize, appendedIndexes);
	else
		loadV1IndexBuffer<uint16_t>(ibuf, offset, previousSize, appendedIndexes);
}

Real VertexIndexToShape::getRadius()
{
	if (mBoundRadius == -1)
	{
		getSize();
		mBoundRadius = std::max(mBounds.x, std::max(mBounds.y, mBounds.z)) * 0.5f;
	}
	return mBoundRadius;
}

Vector3 VertexIndexToShape::getCenterOffset()
{
	getSize();
	return mCenter;
}

Vector3 VertexIndexToShape::getSize()
{
	if (mBounds == Vector3(-1, -1, -1) && getVertexCount())
	{
		auto vmin = mVertexBuffer[0];
		auto vmax = vmin;

		for (const auto vertex : mVertexBuffer)
		{
			vmin.x = std::min(vmin.x, vertex.x);
			vmin.y = std::min(vmin.y, vertex.y);
			vmin.z = std::min(vmin.z, vertex.z);
			vmax.x = std::max(vmax.x, vertex.x);
			vmax.y = std::max(vmax.y, vertex.y);
			vmax.z = std::max(vmax.z, vertex.z);
		}

		mBounds = vmax - vmin;
		mCenter = vmin + mBounds/2;
	}

	return mBounds;
}

//These should be const
const Vector3* VertexIndexToShape::getVertices()
{
	return mVertexBuffer.data();
}

size_t VertexIndexToShape::getVertexCount() const
{
	return mVertexBuffer.size();
}
const unsigned int* VertexIndexToShape::getIndices()
{
	return mIndexBuffer.data();
}

size_t VertexIndexToShape::getIndexCount() const
{
	return mIndexBuffer.size();
}

size_t VertexIndexToShape::getTriangleCount() const
{
	return getIndexCount() / 3;
}

btSphereShape* VertexIndexToShape::createSphere()
{
	const auto rad = getRadius();
	assert((rad > 0.0) &&
		("Sphere radius must be greater than zero"));
	auto shape = new btSphereShape(rad);

	shape->setLocalScaling(Convert::toBullet(mScale));

	return shape;
}

btBoxShape* VertexIndexToShape::createBox()
{
	const auto sz = getSize();

	assert((sz.x > 0.0) && (sz.y > 0.0) && (sz.z > 0.0) &&
		("Size of box must be greater than zero on all axes"));

	auto shape = new btBoxShape(Convert::toBullet(sz * 0.5));

	shape->setLocalScaling(Convert::toBullet(mScale));

	return shape;
}

btCylinderShape* VertexIndexToShape::createCylinder()
{
	const auto sz = getSize()*.5f;

	assert((sz.x > 0.0) && (sz.y > 0.0) && (sz.z > 0.0) &&
		("Size of Cylinder must be greater than zero on all axes"));

	auto biggest = std::max(sz.x, std::max(sz.y, sz.z));

	btCylinderShape* shape;

	//Biggest is X. X is is the priority (was the old behavior of the lib) even if other axis are as "big" as x
	if (biggest == sz.x)
		shape = new btCylinderShapeX(Convert::toBullet(sz));
	//Biggest is Z
	else if (biggest == sz.z)
		shape = new btCylinderShapeZ(Convert::toBullet(sz));
	//Biggest is Y
	else
		shape = new btCylinderShape(Convert::toBullet(sz));

	shape->setLocalScaling(Convert::toBullet(mScale));

	return shape;
}
btConvexHullShape* VertexIndexToShape::createConvex()
{
	assert(getVertexCount() && (getIndexCount() >= 6) &&
		("Mesh must have some vertices and at least 6 indices (2 triangles)"));

	auto shape = new btConvexHullShape{ static_cast<btScalar*>(&mVertexBuffer[0].x), int(getVertexCount()), sizeof(Vector3) };

	shape->setLocalScaling(Convert::toBullet(mScale));

	return shape;
}

btBvhTriangleMeshShape* VertexIndexToShape::createTrimesh()
{
	assert(getVertexCount() && (getIndexCount() >= 6) &&
		("Mesh must have some vertices and at least 6 indices (2 triangles)"));

	const auto numFaces = getTriangleCount();
	auto trimesh = new btTriangleMesh();

	btVector3 vertexPos[3];
	for (auto i = size_t{ 0U }; i < numFaces; ++i)
	{
		for (const auto j : { 0, 1, 2 })  //
			vertexPos[j] = Convert::toBullet2(mVertexBuffer[mIndexBuffer[3 * i + j]]);

		trimesh->addTriangle(vertexPos[0], vertexPos[1], vertexPos[2]);
	}

	const auto useQuantizedAABB = true;
	auto shape = new btBvhTriangleMeshShape(trimesh, useQuantizedAABB);
	//no shape->setLocalScaling(Convert::toBullet(mScale));

	return shape;
}

btCapsuleShape* VertexIndexToShape::createCapsule()
{
	const auto sz = getSize();

	assert((sz.x > 0.0) && (sz.y > 0.0) && (sz.z > 0.0) &&
		("Size of the capsule must be greater than zero on all axes"));

	auto height = std::max(sz.x, std::max(sz.y, sz.z));
	btScalar radius;
	btCapsuleShape* shape;
	// Orient the capsule such that its axiz is aligned with the largest dimension.
	if (height == sz.y)
	{
		radius = std::max(sz.x, sz.z);
		shape = new btCapsuleShape(radius * 0.5f, height * 0.5f);
	}
	else if (height == sz.x)
	{
		radius = std::max(sz.y, sz.z);
		shape = new btCapsuleShapeX(radius * 0.5f, height * 0.5f);
	}
	else
	{
		radius = std::max(sz.x, sz.y);
		shape = new btCapsuleShapeZ(radius * 0.5f, height * 0.5f);
	}

	shape->setLocalScaling(Convert::toBullet(mScale));

	return shape;
}

VertexIndexToShape::~VertexIndexToShape()
{
	if (mBoneIndex)
	{
		for (auto i = begin(*mBoneIndex);
			i != end(*mBoneIndex);
			++i)
			delete i->second;
		delete mBoneIndex;
	}
}

VertexIndexToShape::VertexIndexToShape(const Matrix4 &transform) :
	mBounds(Vector3(-1, -1, -1)),
	mBoundRadius(-1),
	mBoneIndex(nullptr),
	mTransform(transform),
	mScale(1)
{
}

/*
 * =============================================================================================
 * BtOgre::StaticMeshToShapeConverter
 * =============================================================================================
 */

StaticMeshToShapeConverter::StaticMeshToShapeConverter() :
	VertexIndexToShape(),
	mEntity(nullptr),
	mItem(nullptr),
	mNode(nullptr)
{
}

StaticMeshToShapeConverter::StaticMeshToShapeConverter(v1::Entity *entity, const Matrix4 &transform) :
	VertexIndexToShape(transform),
	mEntity(nullptr),
	mItem(nullptr),
	mNode(nullptr)
{
	addEntity(entity, transform);
}

StaticMeshToShapeConverter::StaticMeshToShapeConverter(v1::Mesh *mesh, const Matrix4 &transform) :
	VertexIndexToShape(transform),
	mEntity(nullptr),
	mItem(nullptr),
	mNode(nullptr)
{
	addMesh(mesh, transform);
}

StaticMeshToShapeConverter::StaticMeshToShapeConverter(Item* item, const Matrix4& transform)
{
	addItem(item, transform);
}

StaticMeshToShapeConverter::StaticMeshToShapeConverter(Renderable *rend, const Matrix4 &transform) :
	VertexIndexToShape(transform),
	mEntity(nullptr),
	mItem(nullptr),
	mNode(nullptr)
{
	v1::RenderOperation op;
	rend->getRenderOperation(op, false);
	appendV1VertexData(op.vertexData);
	if (op.useIndexes)
		appendV1IndexData(op.indexData);
}

void StaticMeshToShapeConverter::addEntity(v1::Entity *entity, const Matrix4 &transform)
{
	mEntity = entity;
	mNode = static_cast<SceneNode*>(mEntity->getParentNode());
	mScale = mNode ? mNode->getScale() : Vector3::UNIT_SCALE;

	addMesh(mEntity->getMesh().get(), transform);
}

void StaticMeshToShapeConverter::addMesh(const v1::Mesh *mesh, const Matrix4 &transform)
{
	// Each entity added need to reset size and radius
	// next time getRadius and getSize are asked, they will be computed.
	mBounds = Vector3(-1, -1, -1);
	mBoundRadius = -1;

	mTransform = transform;

	if (mesh->hasSkeleton())
		log("MeshToShapeConverter::addMesh : Mesh " + mesh->getName() + " as skeleton but added to trimesh non animated");

	if (mesh->sharedVertexData[0])
	{
		appendV1VertexData(mesh->sharedVertexData[0]);
	}

	for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
	{
		auto sub_mesh = mesh->getSubMesh(i);

		if (!sub_mesh->useSharedVertices)
		{
			appendV1IndexData(sub_mesh->indexData[0], getVertexCount());
			appendV1VertexData(sub_mesh->vertexData[0]);
		}
		else
		{
			appendV1IndexData(sub_mesh->indexData[0]);
		}
	}
}

void VertexIndexToShape::getV2MeshBufferSize(const Mesh* mesh, size_t& previousVertexSize, size_t& previousIndexSize)
{
	size_t numVertices = 0U;
	size_t numIndices = 0U;

	previousVertexSize = mVertexBuffer.size();
	previousIndexSize = mVertexBuffer.size();

	for (const auto subMesh : mesh->getSubMeshes())
	{
		numVertices += subMesh->mVao[0][0]->getVertexBuffers()[0]->getNumElements();
		numIndices += subMesh->mVao[0][0]->getIndexBuffer()->getNumElements();
	}

	mVertexBuffer.resize(mVertexBuffer.size() + numVertices);
	mIndexBuffer.resize(mIndexBuffer.size() + numIndices);
}

void VertexIndexToShape::extractV2SubMeshVertexBuffer(size_t& subMeshOffset,
	OGRE_VertexArrayObject_ReadRequests requests,
	const size_t& prevSize)
{
	auto subMeshVerticiesNum = requests[0].vertexBuffer->getNumElements();
	switch (requests[0].type)
	{
	case VET_HALF4:
		for (size_t i = 0; i < subMeshVerticiesNum; ++i)
		{
			auto pos = reinterpret_cast<const uint16*>(requests[0].data);	//Stored as 16 bits. Need to use Ogre::Bitwise utilities to extract a floating point form this
			requests[0].data += requests[0].vertexBuffer->getBytesPerElement();
			mVertexBuffer[prevSize + i + subMeshOffset] = mTransform * Vector3{ Bitwise::halfToFloat(pos[0]), Bitwise::halfToFloat(pos[1]), Bitwise::halfToFloat(pos[2]) };
		}
		break;
	case VET_FLOAT3:
		for (size_t i = 0; i < subMeshVerticiesNum; ++i)
		{
			auto pos = reinterpret_cast<const Real*>(requests[0].data);
			requests[0].data += requests[0].vertexBuffer->getBytesPerElement();
			mVertexBuffer[prevSize + i + subMeshOffset] = mTransform * Vector3(pos);
		}
		break;
	default:
		log("Error: Vertex Buffer type not recognised");
	}
	subMeshOffset += subMeshVerticiesNum;
}

void VertexIndexToShape::requestV2VertexBufferFromVao(VertexArrayObject* vao, OGRE_VertexArrayObject_ReadRequests& requests)
{
	requests.push_back(VertexArrayObject::ReadRequests(VES_POSITION));

	vao->readRequests(requests);
	vao->mapAsyncTickets(requests);
}

void VertexIndexToShape::extractV2SubMeshIndexBuffer(const size_t& previousSize, const size_t& offset, const bool& indices32, IndexBufferPacked* indexBuffer)
{
	if (indexBuffer)
	{
		auto asyncTicket = indexBuffer->readRequest(0, indexBuffer->getNumElements());

		if (indices32) loadV2IndexBuffer<uint32>(asyncTicket, offset, previousSize, indexBuffer->getNumElements());
		else loadV2IndexBuffer<uint16>(asyncTicket, offset, previousSize, indexBuffer->getNumElements());
	}
}

void StaticMeshToShapeConverter::addItem(Item* item, const Matrix4& transform)
{
	mItem = item;
	mNode = static_cast<SceneNode*>(mItem->getParentNode());
	mScale = mNode ? mNode->getScale() : Vector3::UNIT_SCALE;

	addMesh(item->getMesh().get(), transform);
}

void StaticMeshToShapeConverter::addMesh(const Mesh* mesh, const Matrix4& transform)
{
	mBounds = Vector3{ -1, -1, -1 };
	mBoundRadius = -1;
	mTransform = transform;

	if (mesh->hasSkeleton())
		log("MeshToShapeConverter::addMesh : Mesh " + mesh->getName() + " as skeleton but added to trimesh non animated");

	//Theses variables will hold the current size of this buffer
	size_t prevVertexSize;
	size_t prevIndexSize;
	size_t appendedIndexes = 0;

	//This will extend the vertex/index buffers to fit the data
	getV2MeshBufferSize(mesh, prevVertexSize, prevIndexSize);

	//The number to offset the index values. When switching submeshes, we want to append the indexes when reconstructing the full thing.
	auto indexOffset = mIndexBuffer.empty() ? 0U : mIndexBuffer[mIndexBuffer.size() - 1];

	//The offset associated with the current submesh
	size_t subMeshOffset = 0U;

	//For each submeshes
	for (const auto& subMesh : mesh->getSubMeshes())
	{
		//Get VAO, go to next if submesh empty
		const auto vaos = subMesh->mVao[0];
		if (vaos.empty()) continue;

		//Get the first LOD level
		const auto vao = vaos[0];

		//Get the packed buffer information
		const auto& vertexBuffers = vao->getVertexBuffers();
		const auto indexBuffer = vao->getIndexBuffer();

		//request async read from buffer
		OGRE_VertexArrayObject_ReadRequests requests;
		requestV2VertexBufferFromVao(vao, requests);	// /!\ Don't forget that this call will map async tickets in the request

		//Load the requested data into vertex buffer, this will map the tickets.
		extractV2SubMeshVertexBuffer(subMeshOffset, requests, prevVertexSize);

		//Don't need that request anymore, unmap all tickets
		vao->unmapAsyncTickets(requests);

		//Read index data
		extractV2SubMeshIndexBuffer(prevIndexSize + appendedIndexes,
			indexOffset,
			vao->getIndexBuffer()->getIndexType() == IndexBufferPacked::IT_32BIT,
			indexBuffer);

		//offset the index values by the number of vertex we got from that VAO
		indexOffset += unsigned(vertexBuffers[0]->getNumElements());
		appendedIndexes += indexBuffer[0].getNumElements();
	}
}

/*
 * =============================================================================================
 * BtOgre::AnimatedMeshToShapeConverter
 * =============================================================================================
 */

AnimatedMeshToShapeConverter::AnimatedMeshToShapeConverter(v1::Entity *entity, const Matrix4 &transform) :
	VertexIndexToShape(transform),
	mEntity(nullptr),
	mNode(nullptr),
	mTransformedVerticesTemp(nullptr),
	mTransformedVerticesTempSize(0)
{
	addEntity(entity, transform);
}

AnimatedMeshToShapeConverter::AnimatedMeshToShapeConverter() :
	VertexIndexToShape(),
	mEntity(nullptr),
	mNode(nullptr),
	mTransformedVerticesTemp(nullptr),
	mTransformedVerticesTempSize(0)
{
}

AnimatedMeshToShapeConverter::~AnimatedMeshToShapeConverter()
{
	delete[] mTransformedVerticesTemp;
}

void AnimatedMeshToShapeConverter::addEntity(v1::Entity *entity, const Matrix4 &transform)
{
	// Each entity added need to reset size and radius
	// next time getRadius and getSize are asked, they're computed.
	mBounds = Vector3(-1, -1, -1);
	mBoundRadius = -1;

	mEntity = entity;
	mNode = static_cast<SceneNode*>(mEntity->getParentNode());
	mTransform = transform;

	assert(entity->getMesh()->hasSkeleton());

	mEntity->addSoftwareAnimationRequest(false);
	mEntity->_updateAnimation();

	if (mEntity->getMesh()->sharedVertexData[0])
	{
		addAnimatedVertexData(mEntity->getMesh()->sharedVertexData[0],
			mEntity->_getSkelAnimVertexData(),
			&mEntity->getMesh()->sharedBlendIndexToBoneIndexMap);
	}

	for (unsigned int i = 0; i < mEntity->getNumSubEntities(); ++i)
	{
		auto sub_mesh = mEntity->getSubEntity(i)->getSubMesh();

		if (!sub_mesh->useSharedVertices)
		{
			appendV1IndexData(sub_mesh->indexData[0], getVertexCount());

			addAnimatedVertexData(sub_mesh->vertexData[0],
				mEntity->getSubEntity(i)->_getSkelAnimVertexData(),
				&sub_mesh->blendIndexToBoneIndexMap);
		}
		else
		{
			appendV1IndexData(sub_mesh->indexData[0]);
		}
	}

	mEntity->removeSoftwareAnimationRequest(false);
}

void AnimatedMeshToShapeConverter::addMesh(const v1::MeshPtr &mesh, const Matrix4 &transform)
{
	// Each entity added need to reset size and radius
	// next time getRadius and getSize are asked, they're computed.
	mBounds = Vector3(-1, -1, -1);
	mBoundRadius = -1;
	mTransform = transform;

	assert(mesh->hasSkeleton());

	if (mesh->sharedVertexData[0])
	{
		addAnimatedVertexData(mesh->sharedVertexData[0],
			nullptr,
			&mesh->sharedBlendIndexToBoneIndexMap);
	}

	for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
	{
		auto sub_mesh = mesh->getSubMesh(i);

		if (!sub_mesh->useSharedVertices)
		{
			appendV1IndexData(sub_mesh->indexData[0], getVertexCount());

			addAnimatedVertexData(sub_mesh->vertexData[0],
				nullptr,
				&sub_mesh->blendIndexToBoneIndexMap);
		}
		else
		{
			appendV1IndexData(sub_mesh->indexData[0]);
		}
	}
}

bool AnimatedMeshToShapeConverter::getBoneVertices(unsigned char bone,
	unsigned int &vertex_count,
	Vector3* &vertices,
	const Vector3 &bonePosition)
{
	auto i = mBoneIndex->find(bone);

	if (i == mBoneIndex->end())
		return false;

	if (i->second->empty())
		return false;

	vertex_count = static_cast<unsigned int>(i->second->size()) + 1;
	if (vertex_count > mTransformedVerticesTempSize)
	{
		if (mTransformedVerticesTemp)
			delete[] mTransformedVerticesTemp;

		mTransformedVerticesTemp = new Vector3[vertex_count];
	}

	vertices = mTransformedVerticesTemp;
	vertices[0] = bonePosition;
	//mEntity->_getParentNodeFullTransform() *
	//	mEntity->getSkeleton()->getBone(bone)->_getDerivedPosition();

	//mEntity->getSkeleton()->getBone(bone)->_getDerivedOrientation()
	unsigned int currBoneVertex = 1;
	auto j = i->second->begin();
	while (j != i->second->end())
	{
		vertices[currBoneVertex] = (*j);
		++j;
		++currBoneVertex;
	}
	return true;
}

btBoxShape* AnimatedMeshToShapeConverter::createAlignedBox(unsigned char bone,
	const Vector3 &bonePosition,
	const Quaternion &boneOrientation)
{
	unsigned int vertex_count;
	Vector3* vertices;

	if (!getBoneVertices(bone, vertex_count, vertices, bonePosition))
		return nullptr;

	auto min_vec(vertices[0]);
	auto max_vec(vertices[0]);

	for (unsigned int j = 1; j < vertex_count; j++)
	{
		min_vec.x = std::min(min_vec.x, vertices[j].x);
		min_vec.y = std::min(min_vec.y, vertices[j].y);
		min_vec.z = std::min(min_vec.z, vertices[j].z);

		max_vec.x = std::max(max_vec.x, vertices[j].x);
		max_vec.y = std::max(max_vec.y, vertices[j].y);
		max_vec.z = std::max(max_vec.z, vertices[j].z);
	}
	const auto maxMinusMin(max_vec - min_vec);
	auto box = new btBoxShape(Convert::toBullet(maxMinusMin));

	/*const Ogre::Vector3 pos
		(min_vec.x + (maxMinusMin.x * 0.5),
		min_vec.y + (maxMinusMin.y * 0.5),
		min_vec.z + (maxMinusMin.z * 0.5));*/

		//box->setPosition(pos);

	return box;
}

bool AnimatedMeshToShapeConverter::getOrientedBox(unsigned char bone,
	const Vector3 &bonePosition,
	const Quaternion &boneOrientation,
	Vector3 &box_afExtent,
	Vector3 *box_akAxis,
	Vector3 &box_kCenter)
{
	unsigned int vertex_count;
	Vector3* vertices;

	if (!getBoneVertices(bone, vertex_count, vertices, bonePosition))
		return false;

	box_kCenter = Vector3::ZERO;

	{
		for (unsigned int c = 0; c < vertex_count; c++)
		{
			box_kCenter += vertices[c];
		}
		const auto invVertexCount = 1.0f / vertex_count;
		box_kCenter *= invVertexCount;
	}
	auto orient = boneOrientation;
	orient.ToAxes(box_akAxis);

	// Let C be the box center and let U0, U1, and U2 be the box axes. Each
	// input point is of the form X = C + y0*U0 + y1*U1 + y2*U2.  The
	// following code computes min(y0), max(y0), min(y1), max(y1), min(y2),
	// and max(y2).  The box center is then adjusted to be
	// C' = C + 0.5*(min(y0)+max(y0))*U0 + 0.5*(min(y1)+max(y1))*U1 +
	//      0.5*(min(y2)+max(y2))*U2

	auto kDiff(vertices[1] - box_kCenter);
	auto fY0Min = kDiff.dotProduct(box_akAxis[0]), fY0Max = fY0Min;
	auto fY1Min = kDiff.dotProduct(box_akAxis[1]), fY1Max = fY1Min;
	auto fY2Min = kDiff.dotProduct(box_akAxis[2]), fY2Max = fY2Min;

	for (unsigned int i = 2; i < vertex_count; i++)
	{
		kDiff = vertices[i] - box_kCenter;

		const auto fY0 = kDiff.dotProduct(box_akAxis[0]);
		if (fY0 < fY0Min)
			fY0Min = fY0;
		else if (fY0 > fY0Max)
			fY0Max = fY0;

		const auto fY1 = kDiff.dotProduct(box_akAxis[1]);
		if (fY1 < fY1Min)
			fY1Min = fY1;
		else if (fY1 > fY1Max)
			fY1Max = fY1;

		const auto fY2 = kDiff.dotProduct(box_akAxis[2]);
		if (fY2 < fY2Min)
			fY2Min = fY2;
		else if (fY2 > fY2Max)
			fY2Max = fY2;
	}

	box_afExtent.x = Real(0.5f)*(fY0Max - fY0Min);
	box_afExtent.y = Real(0.5f)*(fY1Max - fY1Min);
	box_afExtent.z = Real(0.5f)*(fY2Max - fY2Min);

	box_kCenter += 0.5f*(fY0Max + fY0Min)*box_akAxis[0] +
		0.5f*(fY1Max + fY1Min)*box_akAxis[1] +
		0.5f*(fY2Max + fY2Min)*box_akAxis[2];

	box_afExtent *= 2.0f;

	return true;
}

btBoxShape *AnimatedMeshToShapeConverter::createOrientedBox(unsigned char bone,
	const Vector3 &bonePosition,
	const Quaternion &boneOrientation)
{
	Vector3 box_akAxis[3];
	Vector3 box_afExtent;
	Vector3 box_afCenter;

	if (!getOrientedBox(bone, bonePosition, boneOrientation,
		box_afExtent,
		box_akAxis,
		box_afCenter))
		return nullptr;

	auto geom = new btBoxShape(Convert::toBullet(box_afExtent));
	//geom->setOrientation(Quaternion(box_akAxis[0],box_akAxis[1],box_akAxis[2]));
	//geom->setPosition(box_afCenter);
	return geom;
}
