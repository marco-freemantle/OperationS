// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameUserSettings.h"

USGameUserSettings::USGameUserSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MasterVolume = 5.f;
	MouseSensitivity = 5.f;
}

void USGameUserSettings::SetMasterVolume(float NewValue)
{
	MasterVolume = NewValue;
}

void USGameUserSettings::SetMouseSentivity(float NewValue)
{
	MouseSensitivity = NewValue;
}

float USGameUserSettings::GetMasterVolume() const
{
	return MasterVolume;
}

float USGameUserSettings::GetMouseSensitivity() const
{
	return MouseSensitivity;
}

USGameUserSettings* USGameUserSettings::GetSGameUserSettings()
{
	return Cast<USGameUserSettings>(UGameUserSettings::GetGameUserSettings());
}
