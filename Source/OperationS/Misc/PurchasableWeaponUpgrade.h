// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Purchasable.h"
#include "PurchasableWeaponUpgrade.generated.h"

/**
 * 
 */
UCLASS()
class OPERATIONS_API APurchasableWeaponUpgrade : public APurchasable
{
	GENERATED_BODY()

public:

	virtual void MakePurchase(class ASPlayerState* PlayerState) override;

protected:

	UPROPERTY(EditAnywhere)
	USoundBase* PurchaseUpgradeSound;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastUpgradeWeapon(class AHitScanWeapon* WeaponToUpgrade, ASPlayerController* BuyingPlayerController);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayPurchaseUpgradeAudio();
	
};
