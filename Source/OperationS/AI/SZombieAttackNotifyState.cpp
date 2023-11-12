// Fill out your copyright notice in the Description page of Project Settings.


#include "SZombieAttackNotifyState.h"
#include "Operations/AI/SZombie.h"

void USZombieAttackNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    AActor* Owner = MeshComp->GetOwner();

    ASZombie* ZombieActor = Cast<ASZombie>(Owner);

    if (ZombieActor)
    {
        ZombieActor->ServerAttackPlayer();
    }
}

void USZombieAttackNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	

}
