// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Purchasable.generated.h"

UCLASS()
class OPERATIONS_API APurchasable : public AActor
{
	GENERATED_BODY()
	
public:	
	APurchasable();

	void ShowPickupWidget(bool bShowWidget);

	UPROPERTY(VisibleAnywhere, Category = "Purchasable Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, Category = "Purchasable Properties")
	UStaticMeshComponent* PurchasableMesh;

	virtual void MakePurchase(class ASPlayerState* PlayerState);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PriceToPurchase = 250;

public:	

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Purchasable Properties")
	class UWidgetComponent* PickupWidget;

};
