// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerState.h"
#include "OperationS/Character/SCharacter.h"
#include "OperationS/PlayerController/SPlayerController.h"
#include "Net/UnrealNetwork.h"

void ASPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPlayerState, PlayerScore);
	DOREPLIFETIME(ASPlayerState, PlayerKills);
}

void ASPlayerState::AddToScore(float ScoreAmount)
{
	PlayerScore = PlayerScore + ScoreAmount;

	Character = Character == nullptr ? Cast<ASCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ASPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(PlayerScore);
		}
	}
}

void ASPlayerState::OnRep_PlayerScore()
{
	Character = Character == nullptr ? Cast<ASCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ASPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(PlayerScore);
		}
	}
}

void ASPlayerState::AddToKills()
{
	PlayerKills = PlayerKills + 1;
}
