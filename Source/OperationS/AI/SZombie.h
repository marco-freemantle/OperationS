// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Navigation/PathFollowingComponent.h"
#include "OperationS/Interfaces/InteractWithCrosshairsInterface.h"
#include "AITypes.h"
#include "SZombie.generated.h"

UCLASS()
class OPERATIONS_API ASZombie : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASZombie();

	UPROPERTY(VisibleAnywhere)
	class USphereComponent* AttackAreaSphere;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void ServerAttackPlayer();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayAttackSound();

	UPROPERTY(EditAnywhere)
	int32 AttackDamage = 20.f;

	//Player health
	UPROPERTY(EditAnywhere, Category = "Zombie stats")
	float MaxHealth = 100.f;

	UPROPERTY(Replicated, Category = "Zombie stats", VisibleAnywhere, BlueprintReadWrite)
	float Health = 100.f;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(Replicated)
	bool bCanAttack = true;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayAttackMontage();

	TArray<class ASCharacter*> AlivePlayers;

	void UpdateAlivePlayersArray();

	UPROPERTY(Replicated)
	ASCharacter* ClosestCharacter;

	UFUNCTION()
	virtual void OnAttackSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	void FindClosestPlayer();

	void MoveToPlayer();

	void AttackEnded();

	void PlayAttackMontage();

private:

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere)
	UParticleSystem* FleshImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundBase* FleshImpactSound;

public:	
	virtual void Tick(float DeltaTime) override;

};
