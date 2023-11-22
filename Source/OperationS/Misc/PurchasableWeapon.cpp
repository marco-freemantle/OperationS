// Fill out your copyright notice in the Description page of Project Settings.


#include "PurchasableWeapon.h"
#include "Operations/Weapon/Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Operations/PlayerState/SPlayerState.h"
#include "Operations/Character/SCharacter.h"
#include "Operations/SComponents/CombatComponent.h"
#include "Operations/Weapon/WeaponTypes.h"
#include "Kismet/GameplayStatics.h"

APurchasableWeapon::APurchasableWeapon()
{
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	WeaponMesh->SetRenderCustomDepth(true);
}

void APurchasableWeapon::MakePurchase(ASPlayerState* PlayerState)
{
	Super::MakePurchase(PlayerState);

	if (bCanAffordPurchase)
	{
		ASCharacter* BuyingCharacter = Cast<ASCharacter>(PlayerState->GetPlayerController()->GetCharacter());
		if (BuyingCharacter)
		{
			AWeapon* SpawnedWeapon = GetWorld()->SpawnActor<AWeapon>(Weapon);
			if (SpawnedWeapon)
			{
				BuyingCharacter->Combat->EquipWeapon(SpawnedWeapon);
			}
		}
	}
}
