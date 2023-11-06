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

	virtual void OnRep_Score() override;

	void AddToScore(float ScoreAmount);

private:

	class ASCharacter* Character;
	class ASPlayerController* Controller;
};
