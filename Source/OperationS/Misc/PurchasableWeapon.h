// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Purchasable.h"
#include "PurchasableWeapon.generated.h"

/**
 * 
 */
UCLASS()
class OPERATIONS_API APurchasableWeapon : public APurchasable
{
	GENERATED_BODY()
	
public:

	APurchasableWeapon();

	virtual void MakePurchase(class ASPlayerState* PlayerState) override;

	UPROPERTY(VisibleAnywhere, Category = "Purchasable Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> Weapon;
};
