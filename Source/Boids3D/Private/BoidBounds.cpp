// Fill out your copyright notice in the Description page of Project Settings.


#include "BoidBounds.h"
#include "Components/BoxComponent.h"

// Sets default values
ABoidBounds::ABoidBounds()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent")));
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ABoidBounds::BeginPlay()
{
	Super::BeginPlay();

	BoxComponent->bHiddenInGame = false;
}

// Called every frame
void ABoidBounds::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!DebuggingInfo)
	{
		return;
	}

	Super::Tick(DeltaTime);
	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + GetActorForwardVector() * 50.0, FColor::Cyan, true, -1.0, 0U, 3.0);
	DrawDebugLine(GetWorld(), GetActorLocation() + GetActorForwardVector() * 50.0,
		GetActorLocation() + GetActorForwardVector() * 45.0 +
		FVector(0, 0, 5.0), FColor::Cyan, true, -1.0, 0U, 3.0);

	DrawDebugLine(GetWorld(), GetActorLocation() + GetActorForwardVector() * 50.0,
		GetActorLocation() + GetActorForwardVector() * 45.0 +
		FVector(0, 0, -5.0), FColor::Cyan, true, -1.0, 0U, 3.0);

	DrawDebugPoint(GetWorld(), GetMinCorner(), 5.0, FColor::Blue, true, -1.0);
	DrawDebugPoint(GetWorld(), GetMaxCorner(), 5.0, FColor::Red, true, -1.0);
}

FVector ABoidBounds::GetMinCorner()
{
	return GetActorLocation() - BoxComponent->GetScaledBoxExtent();
}

FVector ABoidBounds::GetMaxCorner()
{
	return GetActorLocation() + BoxComponent->GetScaledBoxExtent();
}


FVector ABoidBounds::GetExtents()
{
	return BoxComponent->GetScaledBoxExtent();
}

