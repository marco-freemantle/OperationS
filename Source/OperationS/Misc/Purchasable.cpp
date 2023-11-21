// Fill out your copyright notice in the Description page of Project Settings.


#include "Purchasable.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "OperationS/Character/SCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Operations/PlayerState/SPlayerState.h"

APurchasable::APurchasable()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	PurchasableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PurchasableMesh"));
	SetRootComponent(PurchasableMesh);

	PurchasableMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	PurchasableMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void APurchasable::BeginPlay()
{
	Super::BeginPlay();
	
	//Overlaps are only performed on the server
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &APurchasable::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &APurchasable::OnSphereEndOverlap);
	}
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

//Will only execute on the server
void APurchasable::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASCharacter* SCharacter = Cast<ASCharacter>(OtherActor);
	if (SCharacter)
	{
		SCharacter->SetOverlappingPurchasable(this);
	}
}

//Will only execute on the server
void APurchasable::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ASCharacter* SCharacter = Cast<ASCharacter>(OtherActor);
	if (SCharacter)
	{
		SCharacter->SetOverlappingPurchasable(nullptr);
	}
}

void APurchasable::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

//Only called on the server
void APurchasable::MakePurchase(ASPlayerState* PlayerState)
{
	if (PlayerState && HasAuthority())
	{
		bool bCanAffordPurchase = PlayerState->PlayerScore > PriceToPurchase;
		if (bCanAffordPurchase)
		{
			PlayerState->AddToScore(-PriceToPurchase);
		}
		MulticastPlayPurchaseAudio(bCanAffordPurchase);
	}
}

void APurchasable::MulticastPlayPurchaseAudio_Implementation(bool bCanAffordPurchase)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("CanAfford: %s"), bCanAffordPurchase ? TEXT("true") : TEXT("false")));
	}
	if (!bCanAffordPurchase && PurchaseFailSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PurchaseFailSound, GetActorLocation());
	}
	if (bCanAffordPurchase && PurchaseSuccessSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PurchaseSuccessSound, GetActorLocation());
	}
}

