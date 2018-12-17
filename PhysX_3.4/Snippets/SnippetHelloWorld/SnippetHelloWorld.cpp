// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

// ****************************************************************************
// This snippet illustrates simple use of physx
//
// It creates a number of box stacks on a plane, and if rendering, allows the
// user to create new stacks and fire a ball from the camera position
// ****************************************************************************

#include <ctype.h>

#include "PxPhysicsAPI.h"

#include "../SnippetCommon/SnippetPrint.h"
#include "../SnippetCommon/SnippetPVD.h"
#include "../SnippetUtils/SnippetUtils.h"
#include "model.h"

using namespace physx;

PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;

PxFoundation*			gFoundation = NULL;
PxPhysics*				gPhysics	= NULL;
PxCooking*				gCooking    = NULL;

PxDefaultCpuDispatcher*	gDispatcher = NULL;
PxScene*				gScene		= NULL;

PxMaterial*				gMaterial = NULL;

PxPvd*                  gPvd        = NULL;

PxReal stackZ = 10.0f;

PxRigidDynamic* createDynamic(const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity=PxVec3(0))
{
	PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, t, geometry, *gMaterial, 10.0f);
	dynamic->setAngularDamping(0.5f);
	dynamic->setLinearVelocity(velocity);
	gScene->addActor(*dynamic);
	return dynamic;
}

void createStack(const PxTransform& t, PxU32 size, PxReal halfExtent)
{
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
	for(PxU32 i=0; i<size;i++)
	{
		for(PxU32 j=0;j<size-i;j++)
		{
			PxTransform localTm(PxVec3(PxReal(j*2) - PxReal(size-i), PxReal(i*2+1)+1 + 35, 0) * halfExtent);
			PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
			body->attachShape(*shape);
			PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
			gScene->addActor(*body);
		}
	}
	shape->release();
}


void boxMerge(float verticesBox[40 * 6], float boxSeat[8][3], float boxLeg[4][8][3]) {
	for (int i = 0; i < 40; ++i) {
		if (i < 8) {
			for (int j = 0; j < 3; ++j) verticesBox[i * 6 + j] = boxSeat[i][j];
			verticesBox[i * 6 + 3] = verticesBox[i * 6 + 4] = 0;
			verticesBox[i * 6 + 5] = 1.0;
		}
		else {
			for (int j = 0; j < 3; ++j) verticesBox[i * 6 + j] = boxLeg[i / 8 - 1][i % 8][j];
			verticesBox[i * 6 + 3] = verticesBox[i * 6 + 4] = 0;
			verticesBox[i * 6 + 5] = 1.0;
		}
	}
}
PxConvexMeshGeometry convexGeom[6];


void initPhysics(float* verticesBox, unsigned int* indicesBox, Model &chair, bool interactive){
	for (int i = 1; i < 5; ++i) {
		for (int j = 0; j < 24; ++j) {
			indicesBox[i * 24 + j] = indicesBox[j] + i * 8;
		}
	}
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);

	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport,PxPvdInstrumentationFlag::eALL);

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(),true,gPvd);
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher	= gDispatcher;
	sceneDesc.filterShader	= PxDefaultSimulationFilterShader;
	gScene = gPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if(pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.1f);

	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0,1,0,0), *gMaterial);
	gScene->addActor(*groundPlane);

	for(PxU32 i=0;i<2;i++)
		//createStack(PxTransform(PxVec3(0,0,stackZ-=40.0f)), 5, 10.0f);

	if(!interactive)
		createDynamic(PxTransform(PxVec3(0,40,100)), PxSphereGeometry(10), PxVec3(0,-50,-100));
	//----------------------------------
	/*
	vector<PxVec3> verts;
	vector<PxU32> triangles;
	int nbVertices = 0, nbTriangles = 0;
	chair.meshes.size();
	for (int i = 0; i < chair.meshes.size(); ++i) {
		for (int j = 0; j < chair.meshes[i].vertices.size(); ++j) {
			glm::vec3 &tmp = chair.meshes[i].vertices[j].Position;
			verts.push_back(PxVec3(tmp.x, tmp.y, tmp.z));
		}
		for (int j = 0; j < chair.meshes[i].indices.size(); ++j) {
			triangles.push_back(chair.meshes[i].indices[j] + nbVertices);
		}
		nbVertices += chair.meshes[i].vertices.size();
	}
	PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = nbVertices;
	meshDesc.points.stride = sizeof(PxVec3);
	meshDesc.points.data = &verts[0];
	meshDesc.triangles.count = triangles.size() / 3;
	meshDesc.triangles.stride = 3 * sizeof(PxU32);
	meshDesc.triangles.data = &triangles[0];

	PxDefaultMemoryOutputStream writeBuffer;
	PxTriangleMeshCookingResult::Enum result;
	bool status = gCooking->cookTriangleMesh(meshDesc, writeBuffer, &result);
	if (!status) return ;
	PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	PxTriangleMesh* myTriMesh = gPhysics->createTriangleMesh(readBuffer);
	// create a shape instancing a triangle mesh at the given scale
	PxMeshScale scale(PxVec3(1, 1, 1), PxQuat(PxIdentity));
	PxTriangleMeshGeometry geom(myTriMesh, scale);
	PxTransform localTm(PxVec3(0, 100, 0) * 1);
	PxRigidDynamic* body = gPhysics->createRigidDynamic(localTm);
	
	body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	//body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);
	PxShape* myTriMeshShape = PxRigidActorExt::createExclusiveShape(*body, geom, *gMaterial);
	myTriMeshShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
	body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);
	myTriMeshShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
	
	//PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
	body->setAngularDamping(0.5f);
	body->setLinearVelocity(PxVec3(0, -10, 0));
	gScene->addActor(*body);
	*/



	freopen("resources/chair/vec.txt", "r", stdin);
	float x, y, z;
	vector<PxVec3> vec, convexVerts;
	for (int i = 0; i < 40; ++i) {
		scanf("%f%f%f", &x, &y, &z);
		vec.push_back(PxVec3(x, y, z));
	}
	freopen("resources/chair/index.txt", "r", stdin);
	//int vNumber[6] = { 24, 32, 8, 8, 8, 8 };
	int vNumber[6] = { 8, 8, 8, 8, 8 };

	int cnt = 0;
	for (int i = 0; i < 5; ++i) {
		convexVerts.clear();
		int pos;
		for (int j = 0; j < vNumber[i]; ++j) {
			scanf("%d", &pos);
			PxVec3 tmp = vec[pos - 1];
			//if (i == 1) tmp.y -= 2;
			//if (i > 1) tmp.y -= 100;
			convexVerts.push_back(tmp);
			verticesBox[cnt * 6 + 0] = tmp.x;
			verticesBox[cnt * 6 + 1] = tmp.y;
			verticesBox[cnt * 6 + 2] = tmp.z;
			verticesBox[cnt * 6 + 3] = 0;
			verticesBox[cnt * 6 + 4] = 0;
			verticesBox[cnt * 6 + 5] = 1.0;
			++cnt;
		}
		PxConvexMeshDesc convexDesc;
		PxConvexMesh* convexMesh;
		convexDesc.points.count = vNumber[i];
		convexDesc.points.stride = sizeof(PxVec3);
		convexDesc.points.data = &convexVerts[0];
		convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
		PxDefaultMemoryOutputStream buf;
		PxConvexMeshCookingResult::Enum result2;
		if (!(gCooking->cookConvexMesh(convexDesc, buf, &result2))) return;
		PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
		convexMesh = gPhysics->createConvexMesh(input);
		convexGeom[i] = PxConvexMeshGeometry(convexMesh);
		
		//localPoses.push_back(localTm2);
		//geometries.push_back(&convexGeom);
	}

	/*PxRigidDynamic* PxChair;
	PxChair = PhysXSample::createCompound(inPosition, localPoses, geometries, 0, mManagedMaterials[MATERIAL_YELLOW], gSubMarineDensity)->is<PxRigidDynamic>();

	if (!mSubmarineActor) fatalError("createCompound failed!");

	//disable the current and buoyancy effect for the sub.
	mSubmarineActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);

	// set the filtering group for the submarine
	setupFiltering(mSubmarineActor, FilterGroup::eSUBMARINE, FilterGroup::eMINE_HEAD | FilterGroup::eMINE_LINK);

	mSubmarineActor->setLinearDamping(0.15f);
	mSubmarineActor->setAngularDamping(15.0f);

	PxTransform globalPose;
	globalPose.p = inPosition;
	globalPose.q = PxQuat(yRot, PxVec3(0, 1, 0));
	mSubmarineActor->setGlobalPose(globalPose);

	mSubmarineActor->setCMassLocalPose(PxTransform(PxIdentity));
	
	return mSubmarineActor;*/











	/*for (int i = 0; i < chair.meshes.size(); ++i) {
		for (int j = 0; j < chair.meshes[i].vertices.size(); ++j) {
			glm::vec3 &tmp = chair.meshes[i].vertices[j].Position;
			bool ok = 1;
			for (int k = 0; k < vec.size(); ++k) {
				if (sgn(vec[k].x - tmp.x) == 0 && sgn(vec[k].y - tmp.y) == 0 && sgn(vec[k].z - tmp.z) == 0) {
					ok = 0;
					break;
				}
			}
			if(ok) vec.push_back(PxVec3(tmp.x, tmp.y, tmp.z));
		}
	}*/
	
	//shape->release();



	//-----add chair-----

}

void push(PxVec3 position, PxQuat rotation, PxVec3 velocity) {
	PxTransform localTm2(position, rotation);
	PxRigidDynamic* body = gPhysics->createRigidDynamic(localTm2);
	for (int i = 0; i < 5; ++i) {
		PxShape* convexShape = PxRigidActorExt::createExclusiveShape(*body, convexGeom[i], *gMaterial);
		//PxShape* convexShape = PxRigidActorExt::createExclusiveShape(*body, convexGeom, *gMaterial);
		convexShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		convexShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
	}
	body->setLinearVelocity(velocity);
	PxRigidBodyExt::updateMassAndInertia(*body, 5.0f);
	body->setAngularDamping(0.5f);
	gScene->addActor(*body);
}
void stepPhysics(bool interactive)
{
	PX_UNUSED(interactive);
	gScene->simulate(1.0f/60.0f);
	gScene->fetchResults(true);
}
	
void cleanupPhysics(bool interactive)
{
	PX_UNUSED(interactive);
	gScene->release();
	gDispatcher->release();
	gCooking->release();
	gPhysics->release();	
	PxPvdTransport* transport = gPvd->getTransport();
	gPvd->release();
	transport->release();
	
	gFoundation->release();
	
	printf("SnippetHelloWorld done.\n");
}

void keyPress(unsigned char key, const PxTransform& camera)
{
	switch(toupper(key))
	{
	case 'B':	createStack(PxTransform(PxVec3(0,0,stackZ-=10.0f)), 10, 2.0f);						break;
	case ' ':	createDynamic(camera, PxSphereGeometry(3.0f), camera.rotate(PxVec3(0,0,-1))*200);	break;
	}
}

int snippetMain(int, const char*const*)
{
#ifdef RENDER_SNIPPET
	extern void renderLoop();
	renderLoop();
#else
	static const PxU32 frameCount = 100;
	initPhysics(false);
	for(PxU32 i=0; i<frameCount; i++)
		stepPhysics(false);
	cleanupPhysics(false);
#endif

	return 0;
}
