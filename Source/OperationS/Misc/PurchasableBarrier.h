// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Purchasable.h"
#include "PurchasableBarrier.generated.h"

/**
 * 
 */
UCLASS()
class OPERATIONS_API APurchasableBarrier : public APurchasable
{
	GENERATED_BODY()
	
public:

	APurchasableBarrier();

	virtual void Tick(float DeltaTime) override;

	virtual void MakePurchase(class ASPlayerState* PlayerState) override;

	UPROPERTY(EditAnywhere)
	FVector DesiredBarrierLocation;

protected:

	UFUNCTION(Server, Reliable)
	void ServerMoveBarrier();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayBarrierMoveAudio();

	UPROPERTY(EditAnywhere)
	USoundBase* BarrierMoveSound;

private:

	bool bShouldMove = false;
};
