// Fill out your copyright notice in the Description page of Project Settings.


#include "SAnimInstance.h"
#include "SCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "OperationS/Weapon/Weapon.h"

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

	bHasJumped = SCharacter->bHasJumped;

	if (!bIsInAir)
	{
		SCharacter->bHasJumped = false;
	}

	bIsAccelerating = SCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = SCharacter->IsWeaponEquipped();
	EquippedWeapon = SCharacter->GetEquippedWeapon();
	bIsCrouched = SCharacter->bIsCrouched;
	bIsAiming = SCharacter->IsAiming();
	TurningInPlace = SCharacter->GetTurningInPlace();
	bIsSprinting = SCharacter->GetIsSprinting();
	bElimmed = SCharacter->IsElimmed();

	//Offset yaw for strafing
	FRotator AimRotation = SCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(SCharacter->GetVelocity());
	FRotator DelatRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DelatRot, DeltaTime, 5.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = SCharacter->GetActorRotation();

	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = SCharacter->GetAO_Yaw();
	AO_Pitch = SCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && SCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		SCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
		RightHandRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - SCharacter->GetHitTarget()));

		FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));

	}
}
