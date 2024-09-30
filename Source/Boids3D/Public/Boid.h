// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoidBoundsComponent.h"
#include "BoidContainer.h"
#include "BoidBounds.h"
#include "BoidManager.h"
#include "BoidAvoidanceComponent.h"
#include "Boid.generated.h"

UCLASS()
class BOIDS3D_API ABoid : public AActor
{
	GENERATED_BODY()
private:
	void UpdateMovementFromBounds(FVector& MoveTo);
	void DebugDraw();
	void DebugAvoidance();
	FVector CalculateSeparationHeading(TArray<TObjectPtr<ABoid>>& Neighbors);
	FVector CalculateAlignmentHeading(TArray<TObjectPtr<ABoid>>& Neighbors);
	FVector CalculateCohesionHeading(TArray<TObjectPtr<ABoid>>& Neighbors);
	FVector CalculateObstacleAvoidance();

	TObjectPtr<class UBoidBoundsComponent> Bounds;
public:	
	// Sets default values for this actor's properties
	ABoid();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	FVector Velocity = FVector::Zero();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ABoidContainer> BoidContainer;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ABoidManager> BoidManager;

	UPROPERTY(EditAnywhere)
	bool DebuggingInfo = false;

	UPROPERTY(EditAnywhere)
	bool DebugAvoidanceInfo = false;

	UPROPERTY(EditAnywhere)
	bool FreezeMovement = false;
};
