// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedPickup.h"
#include "Operations/Character/SCharacter.h"
#include "Operations/SComponents/BuffComponent.h"

ASpeedPickup::ASpeedPickup()
{
	bReplicates = true;
}

void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ASCharacter* SCharacter = Cast<ASCharacter>(OtherActor);
	if (SCharacter)
	{
		UBuffComponent* BuffComponent = SCharacter->Buff;
		if (BuffComponent)
		{
			BuffComponent->BuffSpeed(SpeedMultiplier, SpeedBuffTime);
		}
	}
	Destroy();
}