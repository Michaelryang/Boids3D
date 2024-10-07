// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MarchingCubesComponent.h"
#include "MarchingCubesWorld.generated.h"

class UMarchingCubesComponent;
class UBoxComponent;

class FWorldChunkThread : public FMarchingCubesThread {
public:
	FWorldChunkThread(int Index)
	{
		ThreadIndex = Index;
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("FWorldChunkThread Started")));
		Thread = FRunnableThread::Create(this, TEXT("ChunkThread"));
	}

	virtual uint32 Run() override;
	int ThreadIndex;
	FIntVector2 Coords;
	AMarchingCubesWorld* MCWorld;
	void StartCompute(UMarchingCubesComponent* InMarchingCubesComponent, AMarchingCubesWorld* InWorld, FIntVector2 InCoords);
};

USTRUCT(BlueprintType)
struct FChunkMapData {
	GENERATED_BODY()
public:
	bool Contains(int x, int y)
	{
		return ChunkBoxMap.Contains(FIntVector2(x, y));
	}

	void SpawnChunk(int x, int y, AMarchingCubesWorld* ParentActor);
	void DeleteChunksOutOfRange(int x, int y, int ViewDistance, AMarchingCubesWorld* ParentActor);

	TMap<FIntVector2, UMarchingCubesComponent*> ChunkMap;

	UPROPERTY(VisibleAnywhere)
	TMap<FIntVector2, UBoxComponent*> ChunkBoxMap;
};

UCLASS()
class BOIDS3D_API AMarchingCubesWorld : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMarchingCubesWorld();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	FIntVector2 ChunkCoordinatesFromWorldPos(FVector CenterPosition);

	FIntVector2 PlayerCoords;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// CenterPosition is the central position from which the correct chunks will be determined
	void UpdateChunks(FVector CenterPosition);
	void AddToComputedChunkQueue(FIntVector2 Coords, UMarchingCubesComponent* Chunk, int ThreadIndex);
	//virtual void BeginDestroy() override;

	UPROPERTY(EditAnywhere, Category=WorldConfig)
	int ViewDistance = 1; // manhattan distance to chunk (for now)

	UPROPERTY(EditAnywhere, Category = WorldConfig)
	float ChunkCubeSize = 50.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WorldConfig)
	FVector ChunkExtents = FVector(200.0, 200.0, 200.0);

	UPROPERTY(VisibleAnywhere)
	FChunkMapData MapData;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<class UMarchingCubesComponent> MarchingCubesComponent;

	// threads will do the computation from this queue
	TQueue<TPair<FIntVector2, UMarchingCubesComponent*>> ToComputeChunkQueue;

	// threads will add computed data to this queue
	TQueue<TPair<FIntVector2, UMarchingCubesComponent*>> ComputedChunkQueue;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int NumChunkThreads = 8;
	TArray<TPair<bool, FWorldChunkThread*>> ChunkThreads;
};
 