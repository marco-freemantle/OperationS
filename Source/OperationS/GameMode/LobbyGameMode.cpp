// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	if (NumberOfPlayers == PlayersNeededToStartMatch)
	{
        FTimerHandle TravelTimerHandle;
        GetWorldTimerManager().SetTimer(TravelTimerHandle, this, &ThisClass::StartTravel, 1.f, false);
	}
}

void ALobbyGameMode::StartTravel()
{
    UWorld* World = GetWorld();
    if (World)
    {
        bUseSeamlessTravel = true;
        World->ServerTravel(FString("/Game/Maps/MainLevel?listen"));
    }
}
