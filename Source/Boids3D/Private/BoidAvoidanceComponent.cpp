// Fill out your copyright notice in the Description page of Project Settings.


#include "BoidAvoidanceComponent.h"

// Sets default values for this component's properties
UBoidAvoidanceComponent::UBoidAvoidanceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


void UBoidAvoidanceComponent::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void UBoidAvoidanceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	//DrawDebugSphere(GetWorld(), GetComponentLocation(), 100.0, 10, FColor::Magenta);
}

