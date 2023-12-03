// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "SGameUserSettings.generated.h"

/**
 * 
 */
UCLASS()
class OPERATIONS_API USGameUserSettings : public UGameUserSettings
{
	GENERATED_UCLASS_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void SetMasterVolume(float NewValue);

	UFUNCTION(BlueprintCallable)
	void SetMouseSentivity(float NewValue);

	UFUNCTION(BlueprintCallable)
	float GetMasterVolume() const;

	UFUNCTION(BlueprintCallable)
	float GetMouseSensitivity() const;

	UFUNCTION(BlueprintPure)
	static USGameUserSettings* GetSGameUserSettings();

	UPROPERTY(Config, BlueprintReadWrite)
	float MasterVolume;

	UPROPERTY(Config, BlueprintReadWrite)
	float MouseSensitivity;

protected:

	
};
