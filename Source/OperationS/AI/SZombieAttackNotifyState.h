// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SZombieAttackNotifyState.generated.h"

/**
 * 
 */
UCLASS()
class OPERATIONS_API USZombieAttackNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:

	UPROPERTY()
	class ASZombie* Zombie;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
