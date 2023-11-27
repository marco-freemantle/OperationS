// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class OPERATIONS_API ASPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);

	FTimerHandle ImpactCrosshairTimerHandle;
	UFUNCTION(Client, Reliable)
	void ClientSetHUDImpactCrosshair();

	FTimerHandle EliminatedTimerHandle;
	UFUNCTION(Client, Reliable)
	void ClientSetHUDEliminated(const FString& VictimName);

	void SetHUDScore(float Score);

	void SetHUDWeaponAmmo(int32 Ammo, int32 MagCapacity, FString WeaponName);

	void SetHUDMatchCountDown(float CountDownTime);

	UFUNCTION(Client, Reliable)
	void ClientSetHUDKillFeeds(const FString& VictimName, const FString& AttackerName);

	virtual void OnPossess(APawn* InPawn) override;

	virtual void Tick(float DeltaTime) override;

	virtual float GetServerTime(); //Synced with server world clock
	virtual void ReceivedPlayer() override; //Sync with server clock as soon as possible

	void PlayHitSound();

	UPROPERTY(EditAnywhere)
	USoundBase* FleshHitSound;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> KillFeedItemClass;

protected:
	virtual void BeginPlay() override;

	void SetHUDTime();

	/*
	Sync time between client and server
	*/
	//Requests the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	//Reports the current server time to the client in response to the ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	//Difference between client and server time
	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);

private:
	class ASHUD* SHUD;

	float MatchTime = 120.f;
	uint32 CountDownInt = 0;
};
