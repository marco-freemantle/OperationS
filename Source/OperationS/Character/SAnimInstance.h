// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "OperationS/STypes/TurningInPlace.h"
#include "SAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class OPERATIONS_API USAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;
	
	UPROPERTY(BlueprintReadOnly, Category = Character)
	class ASCharacter* SCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool bHasJumped;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool bIsSprinting;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool bWeaponEquipped;

	class AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool bIsAiming;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float YawOffset;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float Lean;

	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float AO_Yaw;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float AO_Pitch;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	FTransform LeftHandTransform;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	ETurningInPlace TurningInPlace;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	FRotator RightHandRotation;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool bElimmed;
};
