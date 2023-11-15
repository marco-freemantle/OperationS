// Fill out your copyright notice in the Description page of Project Settings.


#include "SZombie.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Operations/Character/SCharacter.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "OperationS/AI/SZombieAIController.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "AITypes.h"
#include "Net/UnrealNetwork.h"
#include "Operations/GameMode/SGameMode.h"
#include "Operations/PlayerController/SPlayerController.h"

ASZombie::ASZombie()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	AttackAreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackAreaSphere"));
	AttackAreaSphere->SetupAttachment(RootComponent);
	AttackAreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AttackAreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackAreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	AIControllerClass = ASZombieAIController::StaticClass();
}

void ASZombie::BeginPlay()
{
	Super::BeginPlay();
	
	//Overlaps only performed on the server
	if (HasAuthority())
	{
		AttackAreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AttackAreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AttackAreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ASZombie::OnAttackSphereOverlap);

		FTimerHandle SearchForPlayerTimerHandle;
		GetWorldTimerManager().SetTimer(SearchForPlayerTimerHandle, this, &ThisClass::FindClosestPlayer, 0.5f, true);

		FTimerHandle SearchForAlivePlayersTimerHandle;
		GetWorldTimerManager().SetTimer(SearchForAlivePlayersTimerHandle, this, &ThisClass::UpdateAlivePlayersArray, 1.f, true);

		OnTakeAnyDamage.AddDynamic(this, &ASZombie::ReceiveDamage);

	}
}

void ASZombie::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASZombie, bCanAttack);
	DOREPLIFETIME(ASZombie, ClosestCharacter);
	DOREPLIFETIME(ASZombie, Health);
}

void ASZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASZombie::UpdateAlivePlayersArray()
{
	//Clear the array before updating
	AlivePlayers.Empty();

	//Get all actors of type ASCharacter in the world
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASCharacter::StaticClass(), FoundActors);

	//Iterate through the found actors and add them to the array
	for (AActor* FoundActor : FoundActors)
	{
		ASCharacter* SCharacter = Cast<ASCharacter>(FoundActor);
		if (SCharacter && SCharacter->GetHealth() > 0.f)
		{
			AlivePlayers.Add(SCharacter);
		}
	}
}

void ASZombie::FindClosestPlayer()
{
	if (ClosestCharacter == nullptr && AlivePlayers.Num() > 0)
	{
		ClosestCharacter = AlivePlayers[0];
	}
	if (!ClosestCharacter) return;
	for (AActor* AlivePlayer : AlivePlayers)
	{
		//Check if the overlapping actor is of type ACharacter
		ASCharacter* Player = Cast<ASCharacter>(AlivePlayer);

		if (Player && Player->GetHealth() > 0)
		{
			//Get the location of the current character
			FVector CharacterLocation = Player->GetActorLocation();

			//Calculate distance to current character
			float DistanceToCharacter = FVector::Dist(GetActorLocation(), CharacterLocation);

			//Calculate distance to current closest character
			float DistanceToClosestCharacter = FVector::Dist(GetActorLocation(), ClosestCharacter->GetActorLocation());

			if (DistanceToCharacter < DistanceToClosestCharacter)
			{
				ClosestCharacter = Player;
			}

			MoveToPlayer();
		}
	}
}

void ASZombie::MoveToPlayer()
{
	ASZombieAIController* AIController = Cast<ASZombieAIController>(GetController());
	if (AIController && ClosestCharacter)
	{
		AIController->MoveToActor(ClosestCharacter, 1.f, false);
	}
}

//Will only execute on the server
void ASZombie::OnAttackSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
		FTimerHandle StartAttackTimer;
		GetWorldTimerManager().SetTimer(StartAttackTimer, this, &ThisClass::MulticastPlayAttackMontage, 1.f, true);
}

void ASZombie::MulticastPlayAttackMontage_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage && bCanAttack)
	{
		bCanAttack = false;
		float MontageTime = AnimInstance->Montage_Play(AttackMontage, 1.5f);

		FTimerHandle AttackMontageTimer;
		GetWorldTimerManager().SetTimer(AttackMontageTimer, this, &ThisClass::AttackEnded, MontageTime, false);
	}
}

void ASZombie::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);

	if (Health == 0.f)
	{
		//Zombie dead
		ASGameMode* SGameMode = GetWorld()->GetAuthGameMode<ASGameMode>();
		if (SGameMode)
		{
			ASZombieAIController* ZombieController = Cast<ASZombieAIController>(Controller);
			ASPlayerController* AttackerController = Cast<ASPlayerController>(InstigatorController);
			SGameMode->ZombieEliminated(this, ZombieController, AttackerController);
		}
		MulticastElim();
	}
}

void ASZombie::ServerAttackPlayer_Implementation()
{
	if (ClosestCharacter && HasAuthority())
	{
		if (ClosestCharacter->GetHealth() <= 0)
		{
			ClosestCharacter = nullptr;
			UpdateAlivePlayersArray();
			
			return;
		}

		//Calculate distance to current closest character
		float DistanceToCharacter = FVector::Dist(GetActorLocation(), ClosestCharacter->GetActorLocation());
		if (DistanceToCharacter < 150.f)
		{
			UGameplayStatics::ApplyDamage(ClosestCharacter, AttackDamage, GetController(), this, UDamageType::StaticClass());
			MulticastPlayAttackSound();
		}
	}
}

void ASZombie::MulticastPlayAttackSound_Implementation()
{
	if (!ClosestCharacter) return;
	if (FleshImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FleshImpactParticles, ClosestCharacter->GetActorLocation());
	}
	if (FleshImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FleshImpactSound, ClosestCharacter->GetActorLocation());
	}
}

void ASZombie::AttackEnded()
{
	bCanAttack = true;
}

void ASZombie::MulticastElim_Implementation()
{
	//Disable any character movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	//Turn off collision for the capsule
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//Enable physics simulation on the skeletal mesh to turn into a ragdoll
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	//Set the mesh to ignore bullet collision
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);

	//Clear alive player array
	AlivePlayers.Empty();
	ClosestCharacter = nullptr;

	SetLifeSpan(10.f);
}

