// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OPERATIONS_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class ASCharacter;

	void Heal(float HealAmount, float HealingTime);

	void BuffSpeed(float SpeedMultiplier, float BuffTime);

	void BuffJump(float BuffJumpVelocity, float BuffTime);

	void ReplenishShield(float ShieldAmount, float ReplenishTime);

protected:
	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime);
	void ShieldRampUp(float DeltaTime);

private:
	UPROPERTY()
	class ASCharacter* Character;

	//Heal buff
	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;

	//Shield buff
	bool bReplenishingShield = false;
	float ShieldReplenishRate = 0.f;
	float ShieldReplenishAmount = 0.f;

	//Speed buff
	FTimerHandle SpeedBuffTimer;
	void ResetSpeeds();
	float InitialBaseSpeed;
	float InitialCrouchSpeed;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float SpeedMultiplier);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastResetSpeedBuff(float BaseSpeed, float BaseCrouchSpeed);

	//Jump buff
	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float IntialJumpVelocity;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);
	
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
