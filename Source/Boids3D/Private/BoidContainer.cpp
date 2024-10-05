#include "BoidContainer.h"
#include "BoidBoundsComponent.h"

ABoidContainer::ABoidContainer()
{
	PrimaryActorTick.bCanEverTick = false;
	Bounds = CreateDefaultSubobject<UBoidBoundsComponent>(TEXT("UBoidBoundsComponent"));
	Bounds->SetupAttachment(RootComponent);
}

void ABoidContainer::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABoidContainer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

