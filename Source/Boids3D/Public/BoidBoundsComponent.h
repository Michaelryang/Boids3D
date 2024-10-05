#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "BoidBoundsComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BOIDS3D_API UBoidBoundsComponent : public UBoxComponent
{
	GENERATED_BODY()

public:	
	UBoidBoundsComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FVector GetMinCorner();
	FVector GetMaxCorner();
	FVector GetExtents();

	UPROPERTY(EditAnywhere)
	bool DebuggingInfo = true;
};
