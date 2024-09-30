#include "BoidManager.h"
#include "Boid.h"

ABoidManager::ABoidManager()
{
 	PrimaryActorTick.bCanEverTick = false;
	ComputeAvoidanceRays();
}

void ABoidManager::ComputeAvoidanceRays()
{
	// https://stackoverflow.com/questions/9600801/evenly-distributing-n-points-on-a-sphere/44164075#44164075

	float GoldenRatio = (1 + FMath::Sqrt(5.0)) / 2;
	float AngleIncrement = PI * 2 * GoldenRatio;

	for (int x = 0; x < NumAvoidancePoints; x++) 
	{
		float t = (float)x / NumAvoidancePoints;
		float inclination = FMath::Acos(1 - 2 * t);
		float azimuth = AngleIncrement * x;

		float X = FMath::Sin(inclination) * FMath::Cos(azimuth);
		float Y = FMath::Sin(inclination) * FMath::Sin(azimuth);
		float Z = FMath::Cos(inclination);
		FRotator Rotator = FRotator(-90.0, 0, 0);
		
		AvoidanceRays.Add(Rotator.RotateVector(FVector(X, Y, Z)));
	}
}

void ABoidManager::BeginPlay()
{
	Super::BeginPlay();
}

void ABoidManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

TArray<TObjectPtr<ABoid>> ABoidManager::GetNeighbors(TObjectPtr<ABoid> Boid, float SearchRadius)
{
	TArray<TObjectPtr<ABoid>> Neighbors;
	float RadiusSquared = SearchRadius * SearchRadius;

	for (int x = 0; x < Boids.Num(); ++x)
	{
		float DistanceSquared = FVector::DistSquared(Boids[x]->GetActorLocation(), Boid->GetActorLocation());
		if (Boids[x] != Boid)
		{
			if (DistanceSquared < SearchRadius * SearchRadius)
			{
				Neighbors.Add(Boids[x]);
			}
		}
	}

	return Neighbors;
}


TArray<TObjectPtr<ABoid>> ABoidManager::GetBoidsWithinRadiusFromList(TArray<TObjectPtr<ABoid>>& BoidsList, TObjectPtr<ABoid> Boid, float SearchRadius)
{
	TArray<TObjectPtr<ABoid>> Neighbors;
	for (int x = 0; x < BoidsList.Num(); ++x)
	{
		float DistanceSquared = FVector::DistSquared(BoidsList[x]->GetActorLocation(), Boid->GetActorLocation());
		if (BoidsList[x] != Boid)
		{
			if (DistanceSquared < SearchRadius * SearchRadius)
			{
				Neighbors.Add(BoidsList[x]);
			}
		}
	}

	return Neighbors;
}

void ABoidManager::AddBoid(ABoid* Boid)
{
	Boids.Add(Boid);
}
