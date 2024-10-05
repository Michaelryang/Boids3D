#include "Boid.h"
#include "Components/BoxComponent.h"
#include "BoidBoundsComponent.h"
#include "BoidContainer.h"
#include "BoidManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ABoid::ABoid()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABoid::BeginPlay()
{
	Super::BeginPlay();
	FTransform Transform = GetActorTransform();
	BoidContainer = Cast<ABoidContainer>(UGameplayStatics::GetActorOfClass(this, ABoidContainer::StaticClass()));
	Bounds = BoidContainer->Bounds;

	BoidManager = Cast<ABoidManager>(UGameplayStatics::GetActorOfClass(this, ABoidManager::StaticClass()));

	Velocity = GetActorForwardVector();
	
	if (BoidManager)
	{
		Velocity *= BoidManager->MaxSpeed;
	}

}

void ABoid::UpdateMovementFromBounds(FVector& ToMove)
{
	if (!Bounds)
	{
		return;
	}

	FVector MinCorner = Bounds->GetMinCorner();
	FVector MaxCorner = Bounds->GetMaxCorner();

	if (BoidManager->BoundsBehaviorType == EBoundsBehavior::CLAMP)
	{
		float DeltaX = ToMove.X;
		float DeltaY = ToMove.Y;
		float DeltaZ = ToMove.Z;

		ToMove.X = FMath::Max(ToMove.X, MinCorner.X - GetActorLocation().X);
		ToMove.X = FMath::Min(ToMove.X, MaxCorner.X - GetActorLocation().X);
		ToMove.Y = FMath::Max(ToMove.Y, MinCorner.Y - GetActorLocation().Y);
		ToMove.Y = FMath::Min(ToMove.Y, MaxCorner.Y - GetActorLocation().Y);
		ToMove.Z = FMath::Max(ToMove.Z, MinCorner.Z - GetActorLocation().Z);
		ToMove.Z = FMath::Min(ToMove.Z, MaxCorner.Z - GetActorLocation().Z);
	}
	else if (BoidManager->BoundsBehaviorType == EBoundsBehavior::WRAP)
	{
		FTransform Transform = GetActorTransform();

		if (!UKismetMathLibrary::IsPointInBox(GetActorLocation() + ToMove, BoidContainer->GetActorLocation(), Bounds->GetExtents()))
		{
			FVector Translate = FVector::Zero();
			FVector NewLocation = GetActorLocation() + ToMove;

			if (NewLocation.X < MinCorner.X)
			{
				Translate += FVector(Bounds->GetExtents().X * 2, 0, 0);
			}

			if (NewLocation.X > MaxCorner.X)
			{
				Translate -= FVector(Bounds->GetExtents().X * 2, 0, 0);
			}

			if (NewLocation.Y < MinCorner.Y)
			{
				Translate += FVector(0, Bounds->GetExtents().Y * 2, 0);
			}

			if (NewLocation.Y > MaxCorner.Y)
			{
				Translate -= FVector(0, Bounds->GetExtents().Y * 2, 0);
			}

			if (NewLocation.Z < MinCorner.Z)
			{
				Translate += FVector(0, 0, Bounds->GetExtents().Z * 2);
			}

			if (NewLocation.Z > MaxCorner.Z)
			{
				Translate -= FVector(0, 0, Bounds->GetExtents().Z * 2);
			}

			ToMove += Translate;
		}
	}
}

void ABoid::DebugDraw()
{
	if (!BoidManager)
	{
		return;
	}

	if (BoidManager->UseSeparationRule)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), BoidManager->SeparationSearchRadius, 8, FColor::Cyan, false, -1.0);
	}

	if (BoidManager->UseAlignmentRule)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), BoidManager->AlignmentSearchRadius, 8, FColor::Yellow, false, -1.0);
	}

	if (BoidManager->UseCohesionRule)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), BoidManager->CohesionSearchRadius, 8, FColor::Orange, false, -1.0);
	}

	TArray<TObjectPtr<ABoid>> Neighbors = BoidManager->GetNeighbors(this, BoidManager->MaxSearchRadius());
	
	for (int x = 0; x < Neighbors.Num(); ++x)
	{
		if (FVector::DistSquared(Neighbors[x]->GetActorLocation(), GetActorLocation()) < BoidManager->AlignmentSearchRadius * BoidManager->AlignmentSearchRadius)
		{
			DrawDebugLine(GetWorld(), GetActorLocation(), Neighbors[x]->GetActorLocation(), FColor::Red);
		}
	}

	GEngine->ClearOnScreenDebugMessages();
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Number of neighbors %d"), Neighbors.Num()));

}

void ABoid::DebugAvoidance()
{
	TArray<FVector> Rays = BoidManager->GetAvoidanceRays();
	
	for (int x = 0; x < Rays.Num(); ++x)
	{
		float color = 255.0 * static_cast<float>(x) / static_cast<float>(Rays.Num());

		FVector WorldSpaceRay = GetTransform().GetRotation().RotateVector(Rays[x]);
		float Angle = FMath::Acos(FVector::DotProduct(WorldSpaceRay, GetActorForwardVector()));

		if ( FMath::RadiansToDegrees(Angle) < BoidManager->ViewAngle)
		{
			DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + WorldSpaceRay * BoidManager->AvoidanceViewDistance, FColor::Green);
		}
		else
		{
			DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + WorldSpaceRay * BoidManager->AvoidanceViewDistance, FColor::Red);
		}
	}
}

FVector ABoid::CalculateSeparationHeading(TArray<TObjectPtr<ABoid>>& Neighbors)
{
	FVector AveragePosition = FVector::Zero();

	for (int x = 0; x < Neighbors.Num(); ++x)
	{
		AveragePosition += GetActorLocation() - Neighbors[x]->GetActorLocation();
	}

	return AveragePosition * BoidManager->SeparationFactor;
}


FVector ABoid::CalculateAlignmentHeading(TArray<TObjectPtr<ABoid>>& Neighbors)
{
	FVector AverageVelocity = FVector::Zero();

	for (int x = 0; x < Neighbors.Num(); ++x)
	{
		AverageVelocity += Neighbors[x]->Velocity;
	}

	if (Neighbors.Num() > 0)
	{
		AverageVelocity /= Neighbors.Num();
	}

	return (AverageVelocity - Velocity) * BoidManager->AlignFactor;
}

FVector ABoid::CalculateCohesionHeading(TArray<TObjectPtr<ABoid>>& Neighbors)
{
	FVector AveragePosition = FVector::Zero();

	for (int x = 0; x < Neighbors.Num(); ++x)
	{
		AveragePosition += Neighbors[x]->GetActorLocation();
	}

	if (Neighbors.Num() > 0)
	{
		AveragePosition /= Neighbors.Num();
		return (AveragePosition - GetActorLocation()) * BoidManager->CohesionFactor;
	}

	return FVector::Zero();
}

FVector ABoid::CalculateObstacleAvoidance()
{
	// avoidance rays are at increasing angles from the boid forward direction.
	// the idea is to iterate until a raycast doesn't intersect with an obstacle, and then steer in that direction.
	TArray<FVector> Rays = BoidManager->GetAvoidanceRays();

	FHitResult HitResult;
	float FurthestUnobstructedDistance = 0.0f;
	float ClosestObstructedDistance = std::numeric_limits<float>::max();
	float CloseProximityMultiplier = 1.0;
	FVector BestDirection = GetActorForwardVector();

	ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel1);
	EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::Type::None;
	TArray<AActor*> ActorsToIgnore;

	for (int x = 0; x < Rays.Num(); ++x)
	{
		// match rotation of ray to boid rotation
		FVector WorldSpaceRay = GetTransform().GetRotation().RotateVector(Rays[x]);

		float Angle = FMath::Acos(FVector::DotProduct(WorldSpaceRay, GetActorForwardVector()));

		if (FMath::RadiansToDegrees(Angle) < BoidManager->ViewAngle)
		{
			bool TraceHit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), GetActorLocation(), GetActorLocation() + WorldSpaceRay * BoidManager->AvoidanceViewDistance,
				50.0, TraceChannel, true, ActorsToIgnore, DebugTraceType, HitResult, true);

			if (TraceHit)
			{
				if (DebugAvoidanceInfo)
				{
					DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + WorldSpaceRay * HitResult.Distance, FColor::Yellow, false, 1.0, 2.0);
				}

				// if an obstacle is too close, the avoidance force should be stronger
				if (HitResult.Distance < BoidManager->CloseProximityObstacleRadius)
				{
					if (DebugAvoidanceInfo)
					{
						DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + WorldSpaceRay * HitResult.Distance, FColor::Red, false, 1.0, 2.0);
					}

					CloseProximityMultiplier = BoidManager->CloseProximityObstacleAvoidanceFactor;
				}

				if (HitResult.Distance > FurthestUnobstructedDistance)
				{
					BestDirection = WorldSpaceRay;
					FurthestUnobstructedDistance = HitResult.Distance;
				}
			}
			else
			{
				if (DebugAvoidanceInfo)
				{
					DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + WorldSpaceRay * BoidManager->AvoidanceViewDistance, FColor::Green, false, 1.0, 2.0);
				}
				
				return WorldSpaceRay * BoidManager->MaxSpeed * BoidManager->ObstacleAvoidanceFactor * CloseProximityMultiplier;
			}
		}
	}
	return BestDirection * BoidManager->MaxSpeed * BoidManager->ObstacleAvoidanceFactor * CloseProximityMultiplier;
}


void ABoid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DebuggingInfo)
	{
		DebugDraw();
	}

	if (DebugAvoidanceInfo)
	{
		DebugAvoidance();
	}

	if (FreezeMovement)
	{
		return;
	}
	
	TArray<TObjectPtr<ABoid>> FullNeighborsSet;
	TArray<TObjectPtr<ABoid>> SeparationNeighbors;
	TArray<TObjectPtr<ABoid>> AlignmentNeighbors;
	TArray<TObjectPtr<ABoid>> CohesionNeighbors;
	
	if (BoidManager)
	{
		FullNeighborsSet = BoidManager->GetNeighbors(this, BoidManager->MaxSearchRadius());
	}

	SeparationNeighbors = BoidManager->GetBoidsWithinRadiusFromList(FullNeighborsSet, this, BoidManager->SeparationSearchRadius);
	AlignmentNeighbors = BoidManager->GetBoidsWithinRadiusFromList(FullNeighborsSet, this, BoidManager->AlignmentSearchRadius);
	CohesionNeighbors = BoidManager->GetBoidsWithinRadiusFromList(FullNeighborsSet, this, BoidManager->CohesionSearchRadius);

	FTransform Transform = GetActorTransform();
	
	// calculate change in heading
	FVector Heading = FVector::Zero();
	if (BoidManager->UseSeparationRule)
	{
		Heading += CalculateSeparationHeading(SeparationNeighbors);
	}

	if (BoidManager->UseAlignmentRule)
	{
		Heading += CalculateAlignmentHeading(AlignmentNeighbors);
	}

	if (BoidManager->UseCohesionRule)
	{
		Heading += CalculateCohesionHeading(CohesionNeighbors);
	}
	
	// obstacle avoidance heading
	Heading += CalculateObstacleAvoidance();

	Velocity += Heading;
	
	// clamp speed and velocity
	float Speed = Velocity.Length();

	if (Speed == 0.0)
	{
		Velocity = GetActorForwardVector();
		Speed = 1.0;
	}

	if (Speed < BoidManager->MinSpeed)
	{
		Velocity *= BoidManager->MinSpeed / Speed;
	}
	else if (Speed > BoidManager->MaxSpeed)
	{
		Velocity *= BoidManager->MaxSpeed / Speed;
	}
	

	FVector ToMove = Velocity * DeltaTime;

	// add bounds behavior movement
	UpdateMovementFromBounds(ToMove);

	// move boid and rotate
	Transform = GetActorTransform();
	Transform.AddToTranslation(ToMove);

	FVector NewForward = Velocity.GetSafeNormal();
	FRotator DeltaRotation = FRotationMatrix::MakeFromX(NewForward).Rotator();
	Transform.SetRotation(FQuat::MakeFromRotator(DeltaRotation));
	SetActorTransform(Transform);
}

