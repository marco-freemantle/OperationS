// Fill out your copyright notice in the Description page of Project Settings.


#include "SAnimInstance.h"
#include "SCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void USAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	SCharacter = Cast<ASCharacter>(TryGetPawnOwner());
}

void USAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (SCharacter == nullptr)
	{
		SCharacter = Cast<ASCharacter>(TryGetPawnOwner());
	}
	if (SCharacter == nullptr) return;

	FVector Velocity = SCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = SCharacter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = SCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
}
