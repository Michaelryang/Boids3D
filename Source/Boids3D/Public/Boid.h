// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Boid.generated.h"
class ABoidManager;
class ABoidContainer;

/*
	Represents a single Boid object.

	Read more: https://cs.stanford.edu/people/eroberts/courses/soco/projects/2008-09/modeling-natural-systems/boids.html
*/
UCLASS()
class BOIDS3D_API ABoid : public AActor
{
	GENERATED_BODY()
private:
	// Adjusts parameter movement vector depending on wrap or clamp to stay within bounds
	void UpdateMovementFromBounds(FVector& MoveTo);
	void DebugDraw();
	void DebugAvoidance();
	FVector CalculateSeparationHeading(TArray<TObjectPtr<ABoid>>& Neighbors);
	FVector CalculateAlignmentHeading(TArray<TObjectPtr<ABoid>>& Neighbors);
	FVector CalculateCohesionHeading(TArray<TObjectPtr<ABoid>>& Neighbors);
	FVector CalculateObstacleAvoidance();

	TObjectPtr<class UBoidBoundsComponent> Bounds;
public:	
	ABoid();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	FVector Velocity = FVector::Zero();

	// If a BoidContainer is found in the level, boids will stay within its volume
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ABoidContainer> BoidContainer;

	// ABoidManager maintains a list of all boids in the level
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ABoidManager> BoidManager;

	// RuntimeDebug booleans can be changed at runtime for individual boids, or set in editor for all boids
	UPROPERTY(EditAnywhere, Category = "RuntimeDebug")
	bool DebuggingInfo = false;

	UPROPERTY(EditAnywhere, Category = "RuntimeDebug")
	bool DebugAvoidanceInfo = false;

	UPROPERTY(EditAnywhere, Category = "RuntimeDebug")
	bool FreezeMovement = false;
};
