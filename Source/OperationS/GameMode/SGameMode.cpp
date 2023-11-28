// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameMode.h"
#include "Operations/Character/SCharacter.h"
#include "Operations/PlayerController/SPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "OperationS/PlayerState/SPlayerState.h"
#include "Operations/AI/SZombieAIController.h"
#include "Operations/Weapon/Weapon.h"

void ASGameMode::PlayerEliminated(ASCharacter* ElimmedCharacter, ASPlayerController* VictimController, ASPlayerController* AttackerController, AActor* DamageCauser)
{
	ASPlayerState* AttackerPlayerState = AttackerController ? Cast<ASPlayerState>(AttackerController->PlayerState) : nullptr;
	ASPlayerState* VictimPlayerState = VictimController ? Cast<ASPlayerState>(VictimController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(100.f);

		FString VictimSteamName = VictimController->PlayerState ? VictimController->PlayerState->GetPlayerName() : FString();
		FString AttackerSteamName = AttackerController->PlayerState ? AttackerController->PlayerState->GetPlayerName() : FString();
		AWeapon* WeaponThatKilled = Cast<AWeapon>(DamageCauser);
		AttackerController->ClientSetHUDEliminated(VictimSteamName);

		//Get all player controllers and update KillFeed
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PlayerController = It->Get();
			if (PlayerController)
			{
				ASPlayerController* IndexedController = Cast<ASPlayerController>(PlayerController);
				if (IndexedController && WeaponThatKilled)
				{
					IndexedController->ClientSetHUDKillFeeds(VictimSteamName, AttackerSteamName, WeaponThatKilled->WeaponImage);
				}
			}
		}
	}

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}

void ASGameMode::ZombieEliminated(ASZombie* ElimmedCharacter, ASZombieAIController* VictimController, ASPlayerController* AttackerController)
{
	ASPlayerState* AttackerPlayerState = AttackerController ? Cast<ASPlayerState>(AttackerController->PlayerState) : nullptr;
	ASPlayerState* VictimPlayerState = VictimController ? Cast<ASPlayerState>(VictimController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(100.f);
	}
}

void ASGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}
