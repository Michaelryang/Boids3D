#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoidContainer.generated.h"

/*
	Base actor that maintains a UBoidBoundsComponent that contains all Boids in the level.
	Aquarium_BP is an example of a blueprint class inheriting from this that also adds some glass textures and
	a UMarchingCubesComponent.h
*/
UCLASS()
class BOIDS3D_API ABoidContainer : public AActor
{
	GENERATED_BODY()
	
public:	
	ABoidContainer();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoidBounds)
	TObjectPtr<class UBoidBoundsComponent> Bounds;

};
