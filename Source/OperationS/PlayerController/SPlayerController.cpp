// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerController.h"
#include "Operations/HUD/SHUD.h"
#include "Operations/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Operations/Character/SCharacter.h"

void ASPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SHUD = Cast<ASHUD>(GetHUD());
}

void ASPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ASCharacter* SCharacter = Cast<ASCharacter>(InPawn);

	if (SCharacter)
	{
		SetHUDHealth(SCharacter->GetHealth(), SCharacter->GetMaxHealth());
	}
}

void ASPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	SHUD = SHUD == nullptr ? Cast<ASHUD>(GetHUD()) : SHUD;

	if (SHUD && SHUD->CharacterOverlay && SHUD->CharacterOverlay->HealthBar && SHUD->CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health / MaxHealth;
		SHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		SHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void ASPlayerController::SetHUDScore(float Score)
{
	SHUD = SHUD == nullptr ? Cast<ASHUD>(GetHUD()) : SHUD;

	if (SHUD && SHUD->CharacterOverlay && SHUD->CharacterOverlay->ScoreAmount)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		SHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}

void ASPlayerController::SetHUDWeaponAmmo(int32 Ammo, int32 MagCapacity)
{
	SHUD = SHUD == nullptr ? Cast<ASHUD>(GetHUD()) : SHUD;

	if (SHUD && SHUD->CharacterOverlay && SHUD->CharacterOverlay->WeaponAmmoAmount)
	{
		FString AmmoText = FString::Printf(TEXT("%d/%d"), Ammo, MagCapacity);
		SHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}
