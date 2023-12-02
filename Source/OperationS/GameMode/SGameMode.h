// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SGameMode.generated.h"

USTRUCT(BlueprintType)
struct FPlayerInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(BlueprintReadWrite)
	FString KillsText;

	UPROPERTY(BlueprintReadWrite)
	FString ScoreText;
};

/**
 * 
 */
UCLASS()
class OPERATIONS_API ASGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	virtual void PlayerEliminated(class ASCharacter* ElimmedCharacter, class ASPlayerController* VictimController, ASPlayerController* AttackerController, AActor* DamageCauser);
	virtual void ZombieEliminated(class ASZombie* ElimmedCharacter, class ASZombieAIController* VictimController, ASPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

protected:
	void PostLogin(APlayerController* NewPlayer) override;
	void Logout(AController* Exiting) override;

	UPROPERTY(BlueprintReadWrite)
	TArray<class ASPlayerController*> ConnectedControllers;

	UPROPERTY(BlueprintReadWrite)
	TArray<FPlayerInfo> PlayerInfoArray;

private:
	void UpdateScoreboards();
	void FinishGame(FString AttackerName);
};
