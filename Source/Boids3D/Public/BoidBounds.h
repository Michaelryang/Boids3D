// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoidBounds.generated.h"

UCLASS()
class BOIDS3D_API ABoidBounds : public AActor
{
	GENERATED_BODY()
	
public:	
	ABoidBounds();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = LevelBounds)
	TObjectPtr<class UBoxComponent> BoxComponent;

	FVector GetMinCorner();
	FVector GetMaxCorner();
	FVector GetExtents();

	UPROPERTY(EditAnywhere)
	bool DebuggingInfo = true;
};
