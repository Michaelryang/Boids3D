// Fill out your copyright notice in the Description page of Project Settings.


#include "BoidContainer.h"

// Sets default values
ABoidContainer::ABoidContainer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Bounds = CreateDefaultSubobject<UBoidBoundsComponent>(TEXT("UBoidBoundsComponent"));
	Bounds->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ABoidContainer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABoidContainer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

