// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Operations/Character/SCharacter.h"
#include "Operations/AI/SZombie.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"

void AHitScanWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHitScanWeapon, FireHit);
}

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25f;
		
		//FHitResult FireHit;
		UWorld* World = GetWorld();

		if (World)
		{
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);
			FVector BeamEnd = End;
			if (FireHit.bBlockingHit)
			{
				BeamEnd = FireHit.ImpactPoint;
				ASCharacter* SCharacter = Cast<ASCharacter>(FireHit.GetActor());
				ASZombie* Zombie = Cast<ASZombie>(FireHit.GetActor());

				bool bHitFlesh = (SCharacter || Zombie);

				MulticastPlayHitEffects(bHitFlesh);

				if (SCharacter && HasAuthority() && InstigatorController)
				{
					UGameplayStatics::ApplyDamage(
						SCharacter,
						Damage,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
			}
			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					World,
					BeamParticles,
					SocketTransform
				);
				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
	}
}

void AHitScanWeapon::MulticastPlayHitEffects_Implementation(bool bFleshHit)
{
	if (bFleshHit)
	{
		if (FleshImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FleshImpactParticles, FireHit.ImpactPoint);
		}
		if (FleshImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FleshImpactSound, FireHit.ImpactPoint);
		}
	}
	else
	{
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.ImpactPoint);
		}
		if (ImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, FireHit.ImpactPoint);
		}
	}
}
