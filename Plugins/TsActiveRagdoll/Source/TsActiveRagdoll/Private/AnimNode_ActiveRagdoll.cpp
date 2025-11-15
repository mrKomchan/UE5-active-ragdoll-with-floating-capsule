//#include "AnimBasedRagdoll.h"
#include "AnimNode_ActiveRagdoll.h"
#include "Animation/AnimInstanceProxy.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "Components/SkeletalMeshComponent.h"

#define ANIM_MATH_PI 3.141592724f

FAnimNode_ActiveRagdoll::FAnimNode_ActiveRagdoll()
{
	//mInitialRelativeRotation = FRotator(0.f, 0.f, 0.f);
	//mInitialRelativeTranslation = FVector::ZeroVector;
}


void FAnimNode_ActiveRagdoll::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	mBasePose.Initialize(Context);
};

void FAnimNode_ActiveRagdoll::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	mBasePose.Update(Context);

	mDeltaTime = Context.GetDeltaTime();

	GetEvaluateGraphExposedInputs().Execute(Context);
}

void FAnimNode_ActiveRagdoll::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	mBasePose.CacheBones(Context);
}

void FAnimNode_ActiveRagdoll::EvaluateComponentSpace_AnyThread(FComponentSpacePoseContext& Output)
{
	mBasePose.EvaluateComponentSpace(Output);


	PhysicsTick(mDeltaTime, Output);

}

void FAnimNode_ActiveRagdoll::PhysicsTick(float SubstepDeltaTime, FComponentSpacePoseContext& Output)
{
	const UAnimInstance* const lAnimInstance = Cast<UAnimInstance>(Output.AnimInstanceProxy->GetAnimInstanceObject());

	USkeletalMeshComponent* lSkel = lAnimInstance->GetOwningComponent();

	if (lSkel && lSkel->IsSimulatingPhysics())
	{
		FQuat lBoneQuaternion;
		FQuat lOwnerRotation;

		FVector lBoneTranslation;
		FVector lOwnerTranslation;

		const AActor* const lCharOwner = lAnimInstance->GetOwningActor();

		UPhysicsAsset* PhysicsAsset = lSkel->GetPhysicsAsset();
		if (!PhysicsAsset) return;

		const int32 lPhysicsBoneCount = PhysicsAsset->SkeletalBodySetups.Num();

		if (lCharOwner)
		{
			lOwnerRotation = lCharOwner->GetActorRotation().Quaternion();
			lOwnerTranslation = lCharOwner->GetActorLocation();
		}
		else
		{
			lOwnerRotation = FQuat::Identity;
			lOwnerTranslation = FVector::ZeroVector;
		}

		for (int32 i = 0; i < lPhysicsBoneCount; ++i)
		{
			// กัน nullptr ไว้หน่อย กัน crash เวลา asset บางช่องว่าง
			if (!PhysicsAsset->SkeletalBodySetups[i])
			{
				continue;
			}
			const FName lBoneName = PhysicsAsset->SkeletalBodySetups[i]->BoneName;

			bool IsRoot = false;

			if (lBoneName.IsEqual("pelvis"))
			{
				IsRoot = true;
			}

			FCompactPoseBoneIndex lInd(lSkel->GetBoneIndex(lBoneName));

			lBoneQuaternion = Output.Pose.GetComponentSpaceTransform(lInd).GetRotation();


			lBoneTranslation = Output.Pose.GetComponentSpaceTransform(lInd).GetLocation();



			// Setting up Linear Velocity
			// Updating bone positions in world space
			if (mTranslationWeight > 0.f)
			{
				FVector lTargetPositionInWorld = lOwnerTranslation + lOwnerRotation * (
					(mInitialRelativeTranslation + mInitialRelativeRotation.Quaternion() * lBoneTranslation));

				FVector finalVelocity = (lTargetPositionInWorld - lSkel->GetBoneLocation(lBoneName)) / mDeltaTime;

				if (mTranslationWeight < 1.0f)
				{
					finalVelocity = FMath::Lerp(FVector::ZeroVector, finalVelocity, mTranslationWeight);
				}

				lSkel->SetPhysicsLinearVelocity(finalVelocity, false, lBoneName);
			}


			//Setting up angular velocity
			if (mRotationWeight > 0.f)
			{
				FQuat lTargetRotationInWorld = lOwnerRotation * mInitialRelativeRotation.Quaternion() * lBoneQuaternion;
				FQuat lRotDiff = lTargetRotationInWorld * lSkel->GetBoneQuaternion(lBoneName).Inverse();

				float lAngDiff = 2.0f * FMath::Acos(lRotDiff.W);
				FVector lAngVelocity;

				//Checking for shortest arc.
				if (lAngDiff < ANIM_MATH_PI)
				{
					lAngVelocity = lRotDiff.GetRotationAxis().GetSafeNormal() * lAngDiff / mDeltaTime;
				}
				else
				{
					lAngVelocity = -lRotDiff.GetRotationAxis().GetSafeNormal() * (2.0f * ANIM_MATH_PI - lAngDiff) / mDeltaTime;
				}

				if (mRotationWeight < 1.0f)
				{

					FVector currentAngularVelNormalized = lSkel->GetPhysicsAngularVelocityInRadians(lBoneName).GetSafeNormal();
					FQuat lInterpolatedRot = FQuat::FindBetweenVectors(currentAngularVelNormalized, lAngVelocity.GetSafeNormal());
					lInterpolatedRot = FQuat::Slerp(FQuat::Identity, lInterpolatedRot, mRotationWeight);
					float lRotRad = FMath::Lerp(0.f, lAngVelocity.Size(), mRotationWeight);

					lAngVelocity = lAngVelocity.GetSafeNormal() * lRotRad;

				}


				lSkel->SetPhysicsAngularVelocityInRadians(lAngVelocity, false, lBoneName);


			}
		}
	}

}
