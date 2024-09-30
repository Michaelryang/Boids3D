// Fill out your copyright notice in the Description page of Project Settings.


#include "MarchingCubesWorld.h"

// Sets default values
AMarchingCubesWorld::AMarchingCubesWorld()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent")));

	MarchingCubesComponent = CreateDefaultSubobject<UMarchingCubesComponent>(TEXT("MarchingCubesComponent"));
	MarchingCubesComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AMarchingCubesWorld::BeginPlay()
{
	Super::BeginPlay();
	FVector PlayerLocationWorldPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	PlayerCoords = ChunkCoordinatesFromWorldPos(PlayerLocationWorldPosition);
	UpdateChunks(PlayerLocationWorldPosition);
}

FIntVector2 AMarchingCubesWorld::ChunkCoordinatesFromWorldPos(FVector CenterPosition)
{
	CenterPosition += ChunkExtents;
	CenterPosition.X /= ChunkExtents.X * 2;
	CenterPosition.Y /= ChunkExtents.Y * 2;

	return FIntVector2(FMath::FloorToInt(CenterPosition.X), FMath::FloorToInt(CenterPosition.Y));
}

// Called every frame
void AMarchingCubesWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector PlayerLocationWorldPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	FIntVector2 CurrentCoords = ChunkCoordinatesFromWorldPos(PlayerLocationWorldPosition);

	GEngine->ClearOnScreenDebugMessages();
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("player position %f %f"), PlayerLocationWorldPosition.X, PlayerLocationWorldPosition.Y));
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("player coords %d %d"), PlayerCoords.X, PlayerCoords.Y));


	if (PlayerCoords != CurrentCoords)
	{
		UpdateChunks(PlayerLocationWorldPosition);
		PlayerCoords = CurrentCoords;
		MapData.DeleteChunksOutOfRange(PlayerCoords.X, PlayerCoords.Y, ViewDistance, this);
	}
}

void FChunkMapData::SpawnChunk(int x, int y, AMarchingCubesWorld* ParentActor)
{
	FVector Extents = ParentActor->ChunkExtents;
	FVector WorldPositionOrigin = ParentActor->GetActorLocation();
	
	TObjectPtr<class UBoxComponent> NewBox = Cast<UBoxComponent>(ParentActor->AddComponentByClass(UBoxComponent::StaticClass(), false, FTransform(), false));
	NewBox->SetBoxExtent(Extents);
	NewBox->SetRelativeLocation(FVector(x * Extents.X * 2, y * Extents.Y * 2, 0.0));
	NewBox->SetHiddenInGame(false);
	ChunkBoxMap.Add(FIntVector2(x, y), NewBox);

	TObjectPtr<class UMarchingCubesComponent> NewChunk = Cast<UMarchingCubesComponent>(
		ParentActor->AddComponentByClass(UMarchingCubesComponent::StaticClass(), false, FTransform(), false));

	NewChunk->CubeSize = ParentActor->ChunkCubeSize;
	NewChunk->ContinuousBounds.ZTop = true;
	NewChunk->SurfaceLevelThreshold = 0.65; //TODO: 
	NewChunk->Scale = 400;
	NewChunk->InitVoxelBounds(WorldPositionOrigin + FVector(x * Extents.X * 2, y * Extents.Y * 2, 0.0), WorldPositionOrigin + FVector(x * Extents.X * 2, y * Extents.Y * 2, 0.0) + Extents * 2);
	NewChunk->SetRelativeLocation(FVector(x * Extents.X * 2, y * Extents.Y * 2, 0.0));
	NewChunk->ComputeMarchingCubes();
	ChunkMap.Add(FIntVector2(x, y), NewChunk);
}

void FChunkMapData::DeleteChunksOutOfRange(int x, int y, int ViewDistance, AMarchingCubesWorld* ParentActor)
{
	TSet<FIntVector2> KeySet;
	int32 NumKeys = ChunkMap.GetKeys(KeySet);
	for (FIntVector2 Key : KeySet)
	{
		FIntVector2 DistanceToChunk = Key - FIntVector2(x, y);
		int Distance = FMath::Abs(DistanceToChunk.X) + FMath::Abs(DistanceToChunk.Y);

		if (Distance > ViewDistance)
		{
			ChunkMap[Key]->DestroyComponent();
			ChunkBoxMap[Key]->DestroyComponent();

			ChunkMap.Remove(Key);
			ChunkBoxMap.Remove(Key);
		}
	}
}

void AMarchingCubesWorld::UpdateChunks(FVector CenterPosition)
{
	FIntVector2 CenterCoords = ChunkCoordinatesFromWorldPos(CenterPosition);

	for (int x = -ViewDistance; x <= ViewDistance; ++x)
	{
		for (int y = -ViewDistance; y <= ViewDistance; ++y)
		{
			if (FMath::Abs(x) + FMath::Abs(y) <= ViewDistance)
			{
				if (!MapData.Contains(x + CenterCoords.X, y + CenterCoords.Y))
				{
					MapData.SpawnChunk(x + CenterCoords.X, y + CenterCoords.Y, this);
				}
			}
		}
	}

	
}


