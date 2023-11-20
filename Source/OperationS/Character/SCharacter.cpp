// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "OperationS/Weapon/Weapon.h"
#include "OperationS/SComponents/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "SAnimInstance.h"
#include "OperationS/OperationS.h"
#include "Operations/PlayerController/SPlayerController.h"
#include "Operations/GameMode/SGameMode.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "OperationS/PlayerState/SPlayerState.h"
#include "Components/SpotLightComponent.h"

// Sets default values
ASCharacter::ASCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);


	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	bIsSprinting = false;

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ASCharacter, bHasJumped, COND_SimulatedOnly);
	DOREPLIFETIME(ASCharacter, bIsSprinting);
	DOREPLIFETIME(ASCharacter, Health);
}

void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateHUDHealth();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ASCharacter::ReceiveDamage);
	}
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CameraBoom)
	{
		//Interpolate towards the target offset
		const float InterpSpeed = 10.f;
		CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetCameraOffset, DeltaTime, InterpSpeed);
	}

	AimOffset(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
}

void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ASCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ASCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ASCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASCharacter::SprintButtonPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASCharacter::SprintButtonReleased);
	PlayerInputComponent->BindAction("ToggleLight", IE_Pressed, this, &ASCharacter::ToggleLightButtonPressed);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ASCharacter::GrenadeButtonPressed);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::LookUp);
}

void ASCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

void ASCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ASCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ASCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ASCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		SectionName = FName("Rifle");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ASCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ASCharacter::Elim()
{
	MulticastElim();
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}
	GetWorldTimerManager().SetTimer(ElimTimer, this, &ASCharacter::ElimTimerFinished, ElimDelay);
}

void ASCharacter::MulticastElim_Implementation()
{
	if (SPlayerController)
	{
		SPlayerController->SetHUDWeaponAmmo(0, 0);
	}
	bElimmed = true;

	//Disable any character movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (SPlayerController)
	{
		DisableInput(SPlayerController);
	}

	//Turn off collision for the capsule
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//Enable physics simulation on the skeletal mesh to turn into a ragdoll
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	//Set the mesh to ignore bullet collision
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);

	//Stop camera following ragdoll
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FollowCamera->DetachFromComponent(DetachRules);
}

void ASCharacter::ElimTimerFinished()
{
	ASGameMode* SGameMode = GetWorld()->GetAuthGameMode<ASGameMode>();

	if (SGameMode)
	{
		SGameMode->RequestRespawn(this, Controller);
	}
}

void ASCharacter::MoveForward(float Value)
{
	if (Value < 0 && bIsSprinting && Combat->EquippedWeapon)
	{
		SprintButtonReleased();
	}
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ASCharacter::MoveRight(float Value)
{
	if ((Value == 1.f || Value == -1.f) && bIsSprinting && Combat && Combat->EquippedWeapon)
	{
		SprintButtonReleased();
	}
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ASCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ASCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ASCharacter::EquipButtonPressed()
{
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

void ASCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
		//Set target camera height
		TargetCameraOffset = FVector(0.f, 60.f, 0.f);
	}
	else
	{
		if (!GetCharacterMovement()->IsFalling())
		{
			Crouch();
			//Set target camera height
			TargetCameraOffset = FVector(0.f, 60.f, -40.f);
		}
	}
}

void ASCharacter::ReloadButtonPressed()
{
	if (Combat)
	{
		Combat->Reload();
		if (bIsSprinting && Combat->EquippedWeapon)
		{
			ServerSprintButtonReleased();
		}
	}
}

void ASCharacter::AimButtonPressed()
{
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->SetAiming(true);
	}
}

void ASCharacter::AimButtonReleased()
{
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->SetAiming(false);
	}
}

void ASCharacter::FireButtonPressed()
{
	if (Combat)
	{
		Combat->FireButtonPressed(true);
		SprintButtonReleased();
	}
}

void ASCharacter::FireButtonReleased()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}

}

void ASCharacter::SprintButtonPressed()
{
	//Check the character's movement direction
	FVector ForwardVector = GetActorForwardVector();
	FVector CharacterVelocity = GetCharacterMovement()->Velocity;
	bool bIsMovingForward = FVector::DotProduct(ForwardVector, CharacterVelocity) > 0;

	if (!bIsMovingForward || IsAiming()) return;

	if (GetCombatState() == ECombatState::ECS_Reloading)
	{
		ServerInteruptReload();
	}

	if (HasAuthority())
	{
		GetCharacterMovement()->MaxWalkSpeed = 800.f;
		bIsSprinting = true;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = 800;
		ServerSprintButtonPressed();
	}
}

void ASCharacter::SprintButtonReleased()
{
	if (!bIsSprinting) return;

	bIsSprinting = false;
	if (HasAuthority())
	{
		GetCharacterMovement()->MaxWalkSpeed = 500.f;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = 500.f;
		ServerSprintButtonReleased();
	}
}

void ASCharacter::ServerInteruptReload_Implementation()
{
	MulticastInteruptReload();
}

void ASCharacter::MulticastInteruptReload_Implementation()
{
	//Cancel reload locally before doing it on server
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	UAnimMontage* MontageToStop = AnimInstance->GetCurrentActiveMontage();
	Combat->CombatState = ECombatState::ECS_Unoccupied;
	if (MontageToStop)
	{
		AnimInstance->Montage_Stop(0.2f, MontageToStop);
	}
}

void ASCharacter::ToggleLightButtonPressed()
{
	ServerToggleLightButtonPressed();
}

void ASCharacter::ServerToggleLightButtonPressed_Implementation()
{
	if (HasAuthority() && Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->MulticastToggleLight();
	}
}

void ASCharacter::GrenadeButtonPressed()
{
	if (Combat)
	{
		Combat->ThrowGrenade();
	}
}

void ASCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	//PlayHitReactMontage();
	UpdateHUDHealth();

	if (Health == 0.f)
	{
		ASGameMode* SGameMode = GetWorld()->GetAuthGameMode<ASGameMode>();
		if (SGameMode)
		{
			SPlayerController = SPlayerController == nullptr ? Cast<ASPlayerController>(Controller) : SPlayerController;
			ASPlayerController* AttackerController = Cast<ASPlayerController>(InstigatorController);
			SGameMode->PlayerEliminated(this, SPlayerController, AttackerController);
		}
	}
}

void ASCharacter::ServerSprintButtonPressed_Implementation()
{
	if (HasAuthority())
	{
		bIsSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = 800.f;
	}
}

void ASCharacter::ServerSprintButtonReleased_Implementation()
{
	if (HasAuthority())
	{
		bIsSprinting = false;
		GetCharacterMovement()->MaxWalkSpeed = 500.f;
	}
}

void ASCharacter::Jump()
{
	Super::Jump();

	bHasJumped = true;
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void ASCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;

	FVector Velocity =GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir)
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// Map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ASCharacter::PollInit()
{
	if (SPlayerState == nullptr)
	{
		SPlayerState = GetPlayerState<ASPlayerState>();
		if (SPlayerState)
		{
			SPlayerState->AddToScore(0.f);
		}
	}
}

void ASCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 7.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ASCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled() || !FollowCamera) return;

	FVector CameraToCharacter = FollowCamera->GetComponentLocation() - GetActorLocation();
	if (CameraToCharacter.Size() < CameraThreshold)
	{
		if (GetMesh())
		{
			GetMesh()->SetVisibility(false);
		}

		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->SetOwnerNoSee(true);
		}
	}
	else
	{
		if (GetMesh())
		{
			GetMesh()->SetVisibility(true);
		}

		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->SetOwnerNoSee(false);
		}
	}
}

//Only executed on client since replication only works server->client
void ASCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ASCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat && HasAuthority())
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void ASCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
}

void ASCharacter::UpdateHUDHealth()
{
	SPlayerController = SPlayerController == nullptr ? Cast<ASPlayerController>(Controller) : SPlayerController;
	if (SPlayerController)
	{
		SPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

//This is called from Weapon.cpp, it will only be executed on the server
void ASCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ASCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ASCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

ECombatState ASCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}

AWeapon* ASCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector ASCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}


