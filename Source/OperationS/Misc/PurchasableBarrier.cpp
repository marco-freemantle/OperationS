// Fill out your copyright notice in the Description page of Project Settings.


#include "PurchasableBarrier.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"

APurchasableBarrier::APurchasableBarrier()
{
    PrimaryActorTick.bCanEverTick = true;
}

void APurchasableBarrier::Tick(float DeltaTime)
{
    if (bShouldMove)
    {
        ServerMoveBarrier();
    }
}

void APurchasableBarrier::MakePurchase(ASPlayerState* PlayerState)
{
	Super::MakePurchase(PlayerState);

    if (bCanAffordPurchase)
    {   
        bShouldMove = true;
        MulticastPlayBarrierMoveAudio();
    }
}

void APurchasableBarrier::MulticastPlayBarrierMoveAudio_Implementation()
{
    if (BarrierMoveSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BarrierMoveSound, GetActorLocation());
    }
}

void APurchasableBarrier::ServerMoveBarrier_Implementation()
{
    // Get the current location of the object
    FVector CurrentLocation = GetActorLocation();

    // Interpolate between the current location and the new location over time
    float InterpolationSpeed = 5.0f; // Adjust this value to control the speed of interpolation
    FVector SmoothedLocation = FMath::VInterpTo(CurrentLocation, DesiredBarrierLocation, GetWorld()->GetDeltaSeconds(), InterpolationSpeed);

    // Set the new smoothed location for the object
    SetActorLocation(SmoothedLocation);

    if (CurrentLocation == DesiredBarrierLocation)
    {
        bShouldMove = false;

        //Stop query and hide pickupwidget
        AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
        PickupWidget->SetVisibility(false);
    }
}
