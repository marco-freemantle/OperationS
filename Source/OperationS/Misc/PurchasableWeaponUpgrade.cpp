// Fill out your copyright notice in the Description page of Project Settings.


#include "PurchasableWeaponUpgrade.h"
#include "Kismet/GameplayStatics.h"
#include "Operations/PlayerState/SPlayerState.h"
#include "Operations/Weapon/HitScanWeapon.h"
#include "Operations/Character/SCharacter.h"
#include "Operations/PlayerController/SPlayerController.h"

void APurchasableWeaponUpgrade::MakePurchase(ASPlayerState* PlayerState)
{
	Super::MakePurchase(PlayerState);

	if (bCanAffordPurchase)
	{
		ASCharacter* CharacterOwner = Cast<ASCharacter>(PlayerState->GetPawn());
		if (CharacterOwner)
		{
			AHitScanWeapon* CurrentWeapon = Cast<AHitScanWeapon>(CharacterOwner->GetEquippedWeapon());
			ASPlayerController* BuyingPlayerController = Cast<ASPlayerController>(PlayerState->GetPlayerController());

			if (CurrentWeapon && BuyingPlayerController)
			{
				MulticastUpgradeWeapon(CurrentWeapon, BuyingPlayerController);
			}
			MulticastPlayPurchaseUpgradeAudio();
		}
	}
}

void APurchasableWeaponUpgrade::MulticastUpgradeWeapon_Implementation(AHitScanWeapon* WeaponToUpgrade, ASPlayerController* BuyingPlayerController)
{
	if (WeaponToUpgrade->FireDelay > 0.05)
	{
		WeaponToUpgrade->FireDelay = WeaponToUpgrade->FireDelay - 0.05f;
	}
	if (WeaponToUpgrade->MagCapacity < 40)
	{
		WeaponToUpgrade->MagCapacity = WeaponToUpgrade->MagCapacity + 5;
	}
	if (WeaponToUpgrade->Ammo < 40)
	{
		WeaponToUpgrade->Ammo = WeaponToUpgrade->Ammo + 5;
	}
	if (WeaponToUpgrade->Damage < 50)
	{
		WeaponToUpgrade->Damage = WeaponToUpgrade->Damage + 2.5f;
	}
	WeaponToUpgrade->SetHUDAmmo();
}

void APurchasableWeaponUpgrade::MulticastPlayPurchaseUpgradeAudio_Implementation()
{
	if (PurchaseUpgradeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PurchaseUpgradeSound, GetActorLocation());
	}
}
