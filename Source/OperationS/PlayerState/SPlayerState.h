// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class OPERATIONS_API ASPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void AddToScore(float ScoreAmount);

	UPROPERTY(ReplicatedUsing = OnRep_PlayerScore, BlueprintReadWrite)
	float PlayerScore;

	UFUNCTION()
	void OnRep_PlayerScore();

	void AddToKills();

	UPROPERTY(Replicated, BlueprintReadWrite)
	float PlayerKills;

	FORCEINLINE int GetPlayerScore() const { return PlayerScore; }
	FORCEINLINE int GetPlayerkills() const { return PlayerKills; }

private:

	class ASCharacter* Character;
	class ASPlayerController* Controller;
};
