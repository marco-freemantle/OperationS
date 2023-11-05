// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerController.h"
#include "Operations/HUD/SHUD.h"
#include "Operations/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void ASPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SHUD = Cast<ASHUD>(GetHUD());
}

void ASPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	SHUD = SHUD == nullptr ? Cast<ASHUD>(GetHUD()) : SHUD;

	if (SHUD && SHUD->CharacterOverlay && SHUD->CharacterOverlay->HealthBar && SHUD->CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health / MaxHealth;
		SHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		SHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}
