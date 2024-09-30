// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "BoidBoundsComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BOIDS3D_API UBoidBoundsComponent : public UBoxComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBoidBoundsComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoidBounds)
	//TObjectPtr<class UBoxComponent> BoxComponent;

	FVector GetMinCorner();
	FVector GetMaxCorner();
	FVector GetExtents();

	UPROPERTY(EditAnywhere)
	bool DebuggingInfo = true;
};
