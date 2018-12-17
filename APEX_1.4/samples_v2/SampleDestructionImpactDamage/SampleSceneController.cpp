/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "SampleSceneController.h"
#include "ApexRenderer.h" // for matrix conversion
#include <DirectXMath.h>
#include "XInput.h"
#include "DXUTMisc.h"
#pragma warning(push)
#pragma warning(disable : 4481) // Suppress "nonstandard extension used" warning
#include "DXUTCamera.h"
#pragma warning(pop)

#include "PxPhysicsAPI.h"
#include "PxMath.h"
#include "PsString.h"

#include "ApexResourceCallback.h"
#include "PhysXPrimitive.h"


using namespace physx;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SampleSceneController::SampleSceneController(CFirstPersonCamera* camera, ApexController& apex)
	: mApex(apex), mCamera(camera), mActor(NULL), mLastImpactDamage(0), mCubesCount(0)
{
	mImpactDamageReport.setScene(this);
}

SampleSceneController::~SampleSceneController()
{
}

void SampleSceneController::onSampleStart()
{
	// setup camera
	DirectX::XMVECTORF32 lookAtPt = { 0, 10, 0, 0 };
	DirectX::XMVECTORF32 eyePt = { 0, 20, 60, 0 };
	mCamera->SetViewParams(eyePt, lookAtPt);
	mCamera->SetRotateButtons(false, false, true, false);
	mCamera->SetEnablePositionMovement(true);

	// set physics callback
	mApex.getModuleDestructible()->setImpactDamageReportCallback(&mImpactDamageReport);

	// spawn actor
	mActor = mApex.spawnDestructibleActor("Wall_PC");

	// default destructible params:
	DestructibleParameters params = mActor->getDestructibleParameters();
	params.forceToDamage = 0.01f;
	params.flags &= ~DestructibleParametersFlag::ACCUMULATE_DAMAGE;
	mActor->setDestructibleParameters(params);

	// default cube settings:
	mCubeVelocity = 100;
	mCubeMass = 10;

	// const damage threshold
	DestructibleBehaviorGroupDesc desc;
	mActor->getBehaviorGroup(desc, -1);
	mDamageThreshold = desc.damageThreshold;
}

void SampleSceneController::throwCube()
{
	PxVec3 eyePos = XMVECTORToPxVec4(mCamera->GetEyePt()).getXYZ();
	PxVec3 lookAtPos = XMVECTORToPxVec4(mCamera->GetLookAtPt()).getXYZ();
	PhysXPrimitive* box = mApex.spawnPhysXPrimitiveBox(PxTransform(eyePos));
	PxRigidDynamic* rigidDynamic = box->getActor()->is<PxRigidDynamic>();
	PxRigidBodyExt::setMassAndUpdateInertia(*rigidDynamic, mCubeMass, NULL, false);
	float heaviness = mCubeMass / 1000.0f;
	box->setColor(DirectX::XMFLOAT3(1, 1 - heaviness, 1 - heaviness));

	physx::shdfnd::snprintf(mLastCubeName, 32, "box%d", mCubesCount++);
	rigidDynamic->setName(mLastCubeName);

	PxVec3 dir = (lookAtPos - eyePos).getNormalized();
	rigidDynamic->setLinearVelocity(dir * mCubeVelocity);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SampleSceneController::SampleImpactDamageReport::onImpactDamageNotify(const ImpactDamageEventData* buffer, uint32_t bufferSize)
{
	if (!mScene) return;
	for (uint32_t i = 0; i < bufferSize; i++)
	{
		if (buffer[i].impactDamageActor == NULL || buffer[i].impactDamageActor->getName() == NULL)
			continue;

		if (physx::shdfnd::strcmp(buffer[i].impactDamageActor->getName(), mScene->mLastCubeName) == 0)
		{
			mScene->mLastImpactDamage = buffer[0].damage;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PxFilterFlags DestructionImpactDamageFilterShader(
	PxFilterObjectAttributes attributes0,
	PxFilterData filterData0,
	PxFilterObjectAttributes attributes1,
	PxFilterData filterData1,
	PxPairFlags& pairFlags,
	const void* constantBlock,
	uint32_t constantBlockSize)
{
	PX_UNUSED(constantBlock);
	PX_UNUSED(constantBlockSize);
	// let triggers through
	if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlags();
	}

	if ((PxFilterObjectIsKinematic(attributes0) || PxFilterObjectIsKinematic(attributes1)) &&
		(PxGetFilterObjectType(attributes0) == PxFilterObjectType::eRIGID_STATIC || PxGetFilterObjectType(attributes1) == PxFilterObjectType::eRIGID_STATIC))
	{
		return PxFilterFlag::eSUPPRESS;
	}

	// use a group-based mechanism if the first two filter data words are not 0
	uint32_t f0 = filterData0.word0 | filterData0.word1;
	uint32_t f1 = filterData1.word0 | filterData1.word1;
	if (f0 && f1 && !(filterData0.word0&filterData1.word1 || filterData1.word0&filterData0.word1))
		return PxFilterFlag::eSUPPRESS;

	pairFlags = PxPairFlag::eCONTACT_DEFAULT
		| PxPairFlag::eNOTIFY_CONTACT_POINTS
		| PxPairFlag::eNOTIFY_THRESHOLD_FORCE_PERSISTS
		| PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND
		| PxPairFlag::eNOTIFY_TOUCH_FOUND
		| PxPairFlag::eNOTIFY_TOUCH_PERSISTS;

	// eSOLVE_CONTACT is invalid with kinematic pairs
	if (PxFilterObjectIsKinematic(attributes0) && PxFilterObjectIsKinematic(attributes1))
	{
		pairFlags &= ~PxPairFlag::eSOLVE_CONTACT;
	}

	return PxFilterFlags();
}
