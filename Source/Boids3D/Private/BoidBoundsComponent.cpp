// Fill out your copyright notice in the Description page of Project Settings.


#include "BoidBoundsComponent.h"


// Sets default values for this component's properties
UBoidBoundsComponent::UBoidBoundsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}


void UBoidBoundsComponent::BeginPlay()
{
	Super::BeginPlay();

	//BoxComponent->SetupAttachment(GetAttachmentRoot());
}


void UBoidBoundsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!DebuggingInfo)
	{
		return;
	}

	DrawDebugLine(GetWorld(), GetComponentLocation(), GetComponentLocation() + GetForwardVector() * 50.0, FColor::Cyan, true, -1.0, 0U, 3.0);
	DrawDebugLine(GetWorld(), GetComponentLocation() + GetForwardVector() * 50.0,
		GetComponentLocation() + GetForwardVector() * 45.0 +
		FVector(0, 0, 5.0), FColor::Cyan, true, -1.0, 0U, 3.0);

	DrawDebugLine(GetWorld(), GetComponentLocation() + GetForwardVector() * 50.0,
		GetComponentLocation() + GetForwardVector() * 45.0 +
		FVector(0, 0, -5.0), FColor::Cyan, true, -1.0, 0U, 3.0);

	DrawDebugPoint(GetWorld(), GetMinCorner(), 5.0, FColor::Blue, true, -1.0);
	DrawDebugPoint(GetWorld(), GetMaxCorner(), 5.0, FColor::Red, true, -1.0);
}

FVector UBoidBoundsComponent::GetMinCorner()
{
	return GetComponentLocation() - GetScaledBoxExtent();
}

FVector UBoidBoundsComponent::GetMaxCorner()
{
	return GetComponentLocation() + GetScaledBoxExtent();
}


FVector UBoidBoundsComponent::GetExtents()
{
	return GetScaledBoxExtent();
}