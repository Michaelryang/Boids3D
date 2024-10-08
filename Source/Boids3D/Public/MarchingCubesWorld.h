#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MarchingCubesComponent.h"
#include "MarchingCubesWorld.generated.h"

class UMarchingCubesComponent;
class UBoxComponent;

class FWorldChunkThread : public FMarchingCubesThread {
public:
	// explicit call FMarchingCubesThread(false) to prevent multiple threads from being created
	FWorldChunkThread(int Index) : FMarchingCubesThread(false)
	{
		ThreadIndex = Index;
		Thread = FRunnableThread::Create(this, TEXT("WorldChunkThread"));
	}

	bool Init();
	uint32 Run();
	int ThreadIndex;
	FIntVector2 Coords = FIntVector2::ZeroValue;
	AMarchingCubesWorld* MCWorld = nullptr;
	void StartCompute(UMarchingCubesComponent* InMarchingCubesComponent);
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

	// this is for debugging purposes to show where chunk boundaries are
	UPROPERTY(VisibleAnywhere)
	TMap<FIntVector2, UBoxComponent*> ChunkBoxMap;
};

UCLASS()
class BOIDS3D_API AMarchingCubesWorld : public AActor
{
	GENERATED_BODY()
	
public:	
	AMarchingCubesWorld();

protected:
	virtual void BeginPlay() override;
	FIntVector2 ChunkCoordinatesFromWorldPos(FVector CenterPosition);

	FIntVector2 PlayerCoords = FIntVector2::ZeroValue;
public:	
	virtual void Tick(float DeltaTime) override;

	// CenterPosition is the center of the currently rendered chunks
	void UpdateChunks(FVector CenterPosition);
	void AddToComputedChunkQueue(FIntVector2 Coords, UMarchingCubesComponent* Chunk, int ThreadIndex);

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

	// the key value in the array tracks whether we can assign a new chunk to a thread
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int NumChunkThreads = 8;
	TArray<TPair<bool, FWorldChunkThread*>> ChunkThreads;
};
 