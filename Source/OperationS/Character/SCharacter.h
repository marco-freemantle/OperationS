// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OperationS/STypes/TurningInPlace.h"
#include "OperationS/Interfaces/InteractWithCrosshairsInterface.h"
#include "OperationS/STypes/CombatState.h"
#include "SCharacter.generated.h"

UCLASS()
class OPERATIONS_API ASCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	void PlayFireMontage(bool bAiming);

	void PlayHitReactMontage();

	void PlayElimMontage();

	void PlayReloadMontage();

	void PlayThrowGrenadeMontage();

	void Elim();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void UpdateHUDHealth();

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void ReloadButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed();
	void FireButtonReleased();
	void SprintButtonPressed();
	void SprintButtonReleased();
	void ToggleLightButtonPressed();
	void GrenadeButtonPressed();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	virtual void Jump() override;

	void AimOffset(float DeltaTime);

	//Poll for any relevant classes and initialise HUD
	void PollInit();

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	FVector TargetCameraOffset = FVector(0.f, 60.f, 0.f);

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingPurchasable)
	class APurchasable* OverlappingPurchasable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraShakeBase* CameraShake;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION()
	void OnRep_OverlappingPurchasable(APurchasable* LastPurchasable);

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION(Server, Reliable)
	void ServerSprintButtonPressed();

	UFUNCTION(Server, Reliable)
	void ServerSprintButtonReleased();

	UFUNCTION(Server, Reliable)
	void ServerInteruptReload();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastInteruptReload();

	UFUNCTION(Server, Reliable)
	void ServerToggleLightButtonPressed();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;

	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;

	UPROPERTY(Replicated)
	bool bIsSprinting;

	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 100.f;

	//Player health
	UPROPERTY(EditAnywhere, Category = "Player stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, Category = "Player stats", VisibleAnywhere)
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	class ASPlayerController* SPlayerController;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 5.f;

	void ElimTimerFinished();

	//Grenade
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

public:	

	//Replicated for animation blueprint
	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bHasJumped;

	void SetOverlappingWeapon(AWeapon* Weapon);

	void SetOverlappingPurchasable(APurchasable* Purchasable);

	bool IsWeaponEquipped();

	bool IsAiming();

	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE bool GetIsSprinting() const { return bIsSprinting; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	ECombatState GetCombatState() const;
	AWeapon* GetEquippedWeapon();

	FVector GetHitTarget() const;

	class ASPlayerState* SPlayerState;
};
