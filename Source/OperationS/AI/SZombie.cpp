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

// Sets default values
ASZombie::ASZombie()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	DetectAreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectAreaSphere"));
	DetectAreaSphere->SetupAttachment(RootComponent);
	DetectAreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	DetectAreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DetectAreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

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
		DetectAreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		DetectAreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		DetectAreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ASZombie::OnDetectSphereOverlap);

		AttackAreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AttackAreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AttackAreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ASZombie::OnAttackSphereOverlap);

		FTimerHandle SearchForPlayerTimerHandle;
		GetWorldTimerManager().SetTimer(SearchForPlayerTimerHandle, this, &ThisClass::FindClosestPlayer, 0.5f, true);
	}
}

void ASZombie::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASZombie, bCanAttack);
	DOREPLIFETIME(ASZombie, ClosestCharacter);
}

void ASZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//Will only execute on the server
void ASZombie::OnDetectSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASCharacter* SCharacter = Cast<ASCharacter>(OtherActor);
	if (SCharacter)
	{
		OverlappingCharacters.Add(SCharacter);
	}
}

void ASZombie::FindClosestPlayer()
{
	if (ClosestCharacter == nullptr && OverlappingCharacters.Num() > 0)
	{
		ClosestCharacter = OverlappingCharacters[0];
	}
	if (!ClosestCharacter) return;
	for (AActor* OverlappingActor : OverlappingCharacters)
	{
		//Check if the overlapping actor is of type ACharacter
		ASCharacter* OverlappingCharacter = Cast<ASCharacter>(OverlappingActor);

		if (OverlappingCharacter)
		{
			//Get the location of the overlapping character
			FVector CharacterLocation = OverlappingCharacter->GetActorLocation();

			//Calculate distance to current array index character
			float DistanceToCharacter = FVector::Dist(GetActorLocation(), CharacterLocation);

			//Calculate distance to current closest character
			float DistanceToClosestCharacter = FVector::Dist(GetActorLocation(), ClosestCharacter->GetActorLocation());

			if (DistanceToCharacter < DistanceToClosestCharacter)
			{
				ClosestCharacter = OverlappingCharacter;
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

void ASZombie::ServerAttackPlayer_Implementation()
{
	if (HasAuthority())
	{
		//Calculate distance to current closest character
		float DistanceToCharacter = FVector::Dist(GetActorLocation(), ClosestCharacter->GetActorLocation());
		if (DistanceToCharacter < 150.f)
		{
			UGameplayStatics::ApplyDamage(ClosestCharacter, Damage, GetController(), this, UDamageType::StaticClass());
			MulticastPlayAttackSound();

			//CHECK IF PLAYER IS DEAD, IF SO, REMOVE FROM OVERLAPPING CHARACTERS
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

