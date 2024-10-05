#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoidManager.generated.h"

class ABoid;

UENUM(BlueprintType)
enum class EBoundsBehavior : uint8 {
	CLAMP		UMETA(DisplayName = "Clamp Inside Bounds"),
	WRAP		UMETA(DisplayName = "Wrap Bounds"),
	DISABLED	UMETA(DisplayName = "No Bounds Behavior")
};

UCLASS()
class BOIDS3D_API ABoidManager : public AActor
{
	GENERATED_BODY()
	
public:
	ABoidManager();
private:
	// Array of sphere packing rays. These are in world space.
	TArray<FVector> AvoidanceRays; 
	void ComputeAvoidanceRays();
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	TArray<TObjectPtr<ABoid>> GetNeighbors(TObjectPtr<ABoid> Boid, float SearchRadius);
	TArray<TObjectPtr<ABoid>> GetBoidsWithinRadiusFromList(TArray<TObjectPtr<ABoid>>& BoidsList, TObjectPtr<ABoid> Boid, float SearchRadius);

	float MaxSearchRadius() { return FMath::Max3(SeparationSearchRadius, CohesionSearchRadius, AlignmentSearchRadius); }

	UFUNCTION(BlueprintCallable)
	void AddBoid(ABoid* Boid);
	
	TArray<TObjectPtr<ABoid>> Boids;

	UPROPERTY(EditAnywhere, Category = "Force Tuning")
	float SeparationSearchRadius = 500.0;

	UPROPERTY(EditAnywhere, Category = "Force Tuning")
	float CohesionSearchRadius = 3000.0;

	UPROPERTY(EditAnywhere, Category = "Force Tuning")
	float AlignmentSearchRadius = 1000.0;

	UPROPERTY(EditAnywhere, Category = "Force Tuning")
	float SeparationFactor = 0.02;

	UPROPERTY(EditAnywhere, Category = "Force Tuning")
	float AlignFactor = 0.01;

	UPROPERTY(EditAnywhere, Category = "Force Tuning")
	float CohesionFactor = 0.01;

	UPROPERTY(EditAnywhere, Category = "Force Tuning")
	float ObstacleAvoidanceFactor = 0.3;

	UPROPERTY(EditAnywhere, Category = "Force Tuning")
	float CloseProximityObstacleRadius = 160.0;

	UPROPERTY(EditAnywhere, Category = "Force Tuning")
	float CloseProximityObstacleAvoidanceFactor = 3.0;

	UPROPERTY(EditAnywhere, Category = "Force Tuning")
	bool UseSeparationRule = true;

	UPROPERTY(EditAnywhere, Category = "Force Tuning")
	bool UseAlignmentRule = true;

	UPROPERTY(EditAnywhere, Category = "Force Tuning")
	bool UseCohesionRule = true;

	UPROPERTY(EditAnywhere, Category = "Force Tuning")
	float MinSpeed = 400.0; 
	
	UPROPERTY(EditAnywhere, Category = "Force Tuning")
	float MaxSpeed = 700.0;

	// obstacle avoidance
	UPROPERTY(EditAnywhere, Category="Force Tuning")
	int NumAvoidancePoints = 200;

	UPROPERTY(EditAnywhere, Category = "Force Tuning")
	float AvoidanceViewDistance = 600.0;

	// angle in degrees from forward vector
	UPROPERTY(EditAnywhere, Category = "Force Tuning")
	float ViewAngle = 90.0;

	UPROPERTY(EditAnywhere)
	EBoundsBehavior BoundsBehaviorType = EBoundsBehavior::CLAMP;

	UFUNCTION()
	TArray<FVector>& GetAvoidanceRays() { return AvoidanceRays; }
};
