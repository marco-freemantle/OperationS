// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerController.h"
#include "Operations/HUD/SHUD.h"
#include "Operations/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Operations/Character/SCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"

void ASPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SHUD = Cast<ASHUD>(GetHUD());
}

void ASPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();

	CheckTimeSync(DeltaTime);
}

void ASPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
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
		FString HealthText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Health));
		SHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void ASPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	SHUD = SHUD == nullptr ? Cast<ASHUD>(GetHUD()) : SHUD;

	if (SHUD && SHUD->CharacterOverlay && SHUD->CharacterOverlay->ShieldBar && SHUD->CharacterOverlay->ShieldText)
	{
		const float ShieldPercent = Shield / MaxShield;
		SHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Shield));
		SHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
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

void ASPlayerController::SetHUDWeaponAmmo(int32 Ammo, int32 MagCapacity, UTexture2D* WeaponImage)
{
	SHUD = SHUD == nullptr ? Cast<ASHUD>(GetHUD()) : SHUD;

	if (SHUD && SHUD->CharacterOverlay && SHUD->CharacterOverlay->WeaponAmmoAmount && SHUD->CharacterOverlay->WeaponAmmoLimit && SHUD->CharacterOverlay->WeaponTexture)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		FString AmmoTextLimit = FString::Printf(TEXT("%d"), MagCapacity);
		SHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
		SHUD->CharacterOverlay->WeaponAmmoLimit->SetText(FText::FromString(AmmoTextLimit));
		SHUD->CharacterOverlay->WeaponTexture->SetBrushFromTexture(WeaponImage);
	}
}

void ASPlayerController::SetHUDMatchCountDown(float CountDownTime)
{
	SHUD = SHUD == nullptr ? Cast<ASHUD>(GetHUD()) : SHUD;

	if (SHUD && SHUD->CharacterOverlay && SHUD->CharacterOverlay->MatchCountDownText)
	{
		int32 Minutes = FMath::FloorToInt(CountDownTime / 60.f);
		int32 Seconds = CountDownTime - Minutes * 60;

		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		SHUD->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(CountDownText));
	}
}

void ASPlayerController::ClientSetHUDKillFeeds_Implementation(const FString& VictimName, const FString& AttackerName, UTexture2D* WeaponImage)
{
	SHUD = SHUD == nullptr ? Cast<ASHUD>(GetHUD()) : SHUD;

	if (SHUD && SHUD->CharacterOverlay && SHUD->CharacterOverlay->Killfeeds)
	{
		if (KillFeedItemClass)
		{
			//Create an instance of the Widget Blueprint
			UUserWidget* KillFeedItemWidget = CreateWidget<UUserWidget>(this, KillFeedItemClass);

			UWidgetTree* WidgetTree = KillFeedItemWidget->WidgetTree;

			if (WidgetTree)
			{
				UTextBlock* AttackerTextBlock = WidgetTree->FindWidget<UTextBlock>(TEXT("AttackerText"));
				UTextBlock* VictimTextBlock = WidgetTree->FindWidget<UTextBlock>(TEXT("VictimText"));
				UImage* WeaponImageBlock = WidgetTree->FindWidget<UImage>(TEXT("KillWeaponImage"));

				if (AttackerTextBlock && VictimTextBlock && WeaponImageBlock)
				{
					AttackerTextBlock->SetText(FText::FromString(AttackerName));
					VictimTextBlock->SetText(FText::FromString(VictimName));
					WeaponImageBlock->SetBrushFromTexture(WeaponImage);
					SHUD->CharacterOverlay->Killfeeds->AddChildToVerticalBox(KillFeedItemWidget);

					FTimerHandle DestroyKillFeedItemTimerHandle;
					GetWorldTimerManager().SetTimer(DestroyKillFeedItemTimerHandle, [KillFeedItemWidget]() {
						//Destroy widget after 5 seconds
						KillFeedItemWidget->RemoveFromParent();
						}, 4.f, false);
				}
			}
		}
	}
}

void ASPlayerController::SetHUDTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());

	if (CountDownInt != SecondsLeft)
	{
		SetHUDMatchCountDown(GetServerTime());
	}

	CountDownInt = SecondsLeft;
}



void ASPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ASPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ASPlayerController::GetServerTime()
{
	if (HasAuthority())
	{
		return  GetWorld()->GetTimeSeconds();
	}
	else
	{
		return  GetWorld()->GetTimeSeconds() + ClientServerDelta;
	}
}

void ASPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ASPlayerController::PlayHitSound()
{
	UGameplayStatics::PlaySound2D(this, FleshHitSound, 1.f);
}

void ASPlayerController::ClientSetHUDImpactCrosshair_Implementation()
{
	SHUD = SHUD == nullptr ? Cast<ASHUD>(GetHUD()) : SHUD;

	if (SHUD && SHUD->CharacterOverlay && SHUD->CharacterOverlay->ImpactCrosshair && IsLocalController())
	{
		GetWorldTimerManager().ClearTimer(ImpactCrosshairTimerHandle);
		SHUD->CharacterOverlay->ImpactCrosshair->SetVisibility(ESlateVisibility::Visible);

		GetWorldTimerManager().SetTimer(ImpactCrosshairTimerHandle, [this]() {
			//Set visibility to hidden after 0.75 seconds
			if (SHUD && SHUD->CharacterOverlay && SHUD->CharacterOverlay->ImpactCrosshair)
			{
				SHUD->CharacterOverlay->ImpactCrosshair->SetVisibility(ESlateVisibility::Hidden);
			}
			}, 0.75f, false);
	}
}

void ASPlayerController::ClientSetHUDEliminated_Implementation(const FString& VictimName)
{
	SHUD = SHUD == nullptr ? Cast<ASHUD>(GetHUD()) : SHUD;

	if (SHUD && SHUD->CharacterOverlay && SHUD->CharacterOverlay->Eliminated)
	{
		GetWorldTimerManager().ClearTimer(EliminatedTimerHandle);

		UHorizontalBox* Box = Cast<UHorizontalBox>(SHUD->CharacterOverlay->Eliminated->GetChildAt(0));
		if (Box)
		{
			UTextBlock* VictimNameText = Cast<UTextBlock>(Box->GetChildAt(2));
			if (VictimNameText)
			{
				VictimNameText->SetText(FText::FromString(VictimName));
			}
		}

		SHUD->CharacterOverlay->Eliminated->SetVisibility(ESlateVisibility::Visible);

		GetWorldTimerManager().SetTimer(EliminatedTimerHandle, [this]() {
			//Set visibility to hidden after 3 second
			if (SHUD && SHUD->CharacterOverlay && SHUD->CharacterOverlay->Eliminated)
			{
				SHUD->CharacterOverlay->Eliminated->SetVisibility(ESlateVisibility::Hidden);
			}
			}, 3.f, false);
	}
}
