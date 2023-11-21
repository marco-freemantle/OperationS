// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Operations/Character/SCharacter.h"
#include "Operations/AI/SZombie.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		TMap<ASCharacter*, uint32> CharacterHitMap;
		TMap<ASZombie*, uint32> ZombieHitMap;
		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			WeaponTraceHit(Start, HitTarget, FireHit);

			ASCharacter* SCharacter = Cast<ASCharacter>(FireHit.GetActor());
			ASZombie* Zombie = Cast<ASZombie>(FireHit.GetActor());

			bool bHitFlesh = (SCharacter || Zombie);

			MulticastPlayHitEffects(bHitFlesh, SocketTransform);

			if (HasAuthority() && InstigatorController)
			{
				if (SCharacter)
				{
					if (CharacterHitMap.Contains(SCharacter))
					{
						CharacterHitMap[SCharacter]++;
					}
					else
					{
						CharacterHitMap.Emplace(SCharacter, 1);
					}
				}
				if (Zombie)
				{
					if (ZombieHitMap.Contains(Zombie))
					{
						ZombieHitMap[Zombie]++;
					}
					else
					{
						ZombieHitMap.Emplace(Zombie, 1);
					}
				}
			}
		}
		for (auto HitPair : CharacterHitMap)
		{
			if (HasAuthority() && InstigatorController && HitPair.Key)
			{
				UGameplayStatics::ApplyDamage(
					HitPair.Key,
					Damage * HitPair.Value,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
		for (auto HitPair : ZombieHitMap)
		{
			if (HasAuthority() && InstigatorController && HitPair.Key)
			{
				UGameplayStatics::ApplyDamage(
					HitPair.Key,
					Damage * HitPair.Value,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
	}
}
