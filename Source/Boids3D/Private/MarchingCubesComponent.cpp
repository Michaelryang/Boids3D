// Fill out your copyright notice in the Description page of Project Settings.


#include "MarchingCubesComponent.h"
\
FVector GetPositionFromIndices(int x, int y, int z, FVector MinCornerWorldSpace, float CubeSize)
{
	return MinCornerWorldSpace + FVector(x, y, z) * CubeSize;
}

bool PointIsOnContinuousBoundary(int x, int y, int z, FContinuousBounds& ContinuousBounds, FVoxelGrid& VGrid)
{
	return
		(x == 0 && ContinuousBounds.XBack) ||
		(y == 0 && ContinuousBounds.YLeft) ||
		(z == 0 && ContinuousBounds.ZBottom) ||
		(x == VGrid.XResolution - 1 && ContinuousBounds.XFront) ||
		(y == VGrid.YResolution - 1 && ContinuousBounds.YRight) ||
		(z == VGrid.ZResolution - 1 && ContinuousBounds.ZTop);
}

void UMarchingCubesComponent::BeginPlay()
{
	Super::BeginPlay();
	RealtimeMesh = InitializeRealtimeMesh<URealtimeMeshSimple>();
	SetRealtimeMesh(RealtimeMesh);

	RealtimeMesh->SetupMaterialSlot(0, "PrimaryMaterial", MarchingCubesMaterial);
	RealtimeMesh->SetupMaterialSlot(1, "SecondaryMaterial", MarchingCubesMaterial);
}

void UMarchingCubesComponent::DrawDebugVoxels()
{
	FColor SampleColor;

	for (float x = 0; x < VGrid.XResolution; ++x)
	{
		for (float y = 0; y < VGrid.YResolution; ++y)
		{
			for (float z = 0; z < VGrid.ZResolution; ++z)
			{
				float ColorValue = VGrid.Get(x, y, z) * 255.0;
				SampleColor = FColor(ColorValue, ColorValue, ColorValue);

				if (!ShowPointsBelowThreshold && VGrid.Get(x, y, z) < SurfaceLevelThreshold)
				{
					continue;
				}
				DrawDebugPoint(GetWorld(), GetPositionFromIndices(x, y, z, FVector(MinCornerWorldSpace), CubeSize), 10.0, SampleColor);
			}
		}
	}
	DrawDebugPoint(GetWorld(), FVector(MinCornerWorldSpace) - FVector::One(), 5.0, FColor::Green);
	DrawDebugPoint(GetWorld(), FVector(MaxCornerWorldSpace) + FVector::One(), 5.0, FColor::Blue);
	
}

void UMarchingCubesComponent::InitVoxelBounds(FVector BoundsMinCorner, FVector BoundsMaxCorner)
{
	MinCornerWorldSpace = FVector3f(BoundsMinCorner);
	MaxCornerWorldSpace = FVector3f(BoundsMaxCorner);
	ScaledBoxExtents = (MaxCornerWorldSpace - MinCornerWorldSpace) / 2;
}


void UMarchingCubesComponent::ComputeMarchingCubes()
{
	FMarchingCubesThread* Thread = new FMarchingCubesThread(true);
	Thread->StartCompute(this);
}

bool FMarchingCubesThread::Init() {
	return true;
}

uint32 FMarchingCubesThread::Run()
{
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Thread Started")));
	while (!bShutdown)
	{
		if (!bIsComputing)
		{
			FPlatformProcess::Sleep(0.01);
		}
		else
		{
			Triangles.Empty();
			StreamSet = FRealtimeMeshStreamSet();
			ThreadedComputeVoxelGrid();
			ThreadedComputeTriangles();
			ThreadedMakeStreamSet();
			ThreadedMakeMesh();

			bIsComputing = false;
		}
	}

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Thread Complete")));
	return 0;
}

void FMarchingCubesThread::Exit() {
}

void FMarchingCubesThread::Stop() {
	bShutdown = true;
}

void FMarchingCubesThread::StartCompute(UMarchingCubesComponent* InMarchingCubesComponent)
{
	MarchingCubesComponent = InMarchingCubesComponent;
	RMC = MarchingCubesComponent->RealtimeMesh;
	ScaledBoxExtents = MarchingCubesComponent->ScaledBoxExtents;
	CubeSize = MarchingCubesComponent->CubeSize;
	Scale = MarchingCubesComponent->Scale;
	SurfaceLevelThreshold = MarchingCubesComponent->SurfaceLevelThreshold;
	MinCornerWorldSpace = MarchingCubesComponent->MinCornerWorldSpace;
	MaxCornerWorldSpace = MarchingCubesComponent->MaxCornerWorldSpace;
	MarchingCubesMaterial = MarchingCubesComponent->MarchingCubesMaterial;
	ContinuousBounds = MarchingCubesComponent->ContinuousBounds;
	VGrid = &MarchingCubesComponent->VGrid;
	bIsComputing = true;
}

void FMarchingCubesThread::ThreadedComputeVoxelGrid()
{
	double t1 = FPlatformTime::Seconds();
	*VGrid = FVoxelGrid(
		int((MaxCornerWorldSpace.X - MinCornerWorldSpace.X) / CubeSize),
		int((MaxCornerWorldSpace.Y - MinCornerWorldSpace.Y) / CubeSize),
		int((MaxCornerWorldSpace.Z - MinCornerWorldSpace.Z) / CubeSize));
	float NoiseSample;

	for (float x = 0; x < VGrid->XResolution; ++x)
	{
		for (float y = 0; y < VGrid->YResolution; ++y)
		{
			for (float z = 0; z < VGrid->ZResolution; ++z)
			{
				FVector PositionWorldSpace = GetPositionFromIndices(x, y, z, FVector(MinCornerWorldSpace), CubeSize);
				NoiseSample = FMath::PerlinNoise3D(PositionWorldSpace / Scale);

				VGrid->Set((NoiseSample + 1.0) / 2.0, x, y, z);
			}
		}
	}

	double t2 = FPlatformTime::Seconds();
	double delta = t2 - t1;

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Thread Voxel Time To Compute: %f"), delta));
}

void FMarchingCubesThread::ThreadedComputeTriangles()
{
	double t1 = FPlatformTime::Seconds();

	for (float x = 0; x < VGrid->XResolution - 1; ++x)
	{
		for (float y = 0; y < VGrid->YResolution - 1; ++y)
		{
			for (float z = 0; z < VGrid->ZResolution - 1; ++z)
			{
				int TriangulationIndex = 0;

				TArray<float> CubeData;

				for (int i = 0; i < 8; ++i)
				{
					if (PointIsOnContinuousBoundary(CubePoints[i][0] + x, CubePoints[i][1] + y, CubePoints[i][2] + z, ContinuousBounds, *VGrid))
					{
						CubeData.Add(0.0);
					}
					else
					{
						CubeData.Add(VGrid->Get(CubePoints[i][0] + x, CubePoints[i][1] + y, CubePoints[i][2] + z));
					}
				}

				for (int i = 0; i < 8; ++i)
				{
					if (CubeData[i] > SurfaceLevelThreshold)
					{
						TriangulationIndex |= 1 << i;
					}
				}

				int i = 0;
				while (i < 16)
				{
					if (TriTable[TriangulationIndex][i] == -1)
					{
						break;
					}
					Triangles.Add(TArray<FVector3f>());
					int TriangleEdgeIndices[3] = {
						TriTable[TriangulationIndex][i],
						TriTable[TriangulationIndex][i + 1],
						TriTable[TriangulationIndex][i + 2] };

					for (int j = 0; j < 3; ++j)
					{
						++i;
						int IndexA = EdgeIndices[TriangleEdgeIndices[j]][0];
						int IndexB = EdgeIndices[TriangleEdgeIndices[j]][1];

						FVector3f VertexCubePosition = (CubePointVectors[IndexA] + CubePointVectors[IndexB]) / 2.0 * CubeSize;
						FVector3f VertexLocalPosition = VertexCubePosition - ScaledBoxExtents + FVector3f(x, y, z) * CubeSize;
						Triangles[Triangles.Num() - 1].Add(VertexLocalPosition);
					}
				}
			}
		}
	}
	double t2 = FPlatformTime::Seconds();
	double delta = t2 - t1;
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Mesh Triangles Time To Compute: %f"), delta));
}

void FMarchingCubesThread::ThreadedMakeStreamSet()
{
	double t1 = FPlatformTime::Seconds();
	TRealtimeMeshBuilderLocal<uint32, FPackedNormal, FVector2DHalf, 1> Builder(StreamSet);

	Builder.EnableTangents();
	Builder.EnableTexCoords();
	Builder.EnableColors();
	Builder.EnablePolyGroups();

	for (int i = 0; i < Triangles.Num(); ++i)
	{
		FVector3f Edge1 = Triangles[i][0] - Triangles[i][1];
		FVector3f Edge2 = Triangles[i][0] - Triangles[i][2];
		FVector3f Normal = FVector3f::CrossProduct(Edge2, Edge1);
		FVector3f Tangent = Edge2;

		int32 V0 = Builder.AddVertex(Triangles[i][0])
			.SetNormalAndTangent(Normal, Tangent)
			.SetColor(FColor::Red)
			.SetTexCoord(FVector2f(0.0f, 0.0f));

		int32 V1 = Builder.AddVertex(Triangles[i][1])
			.SetNormalAndTangent(Normal, Tangent)
			.SetColor(FColor::Green)
			.SetTexCoord(FVector2f(0.5f, 1.0f));

		int32 V2 = Builder.AddVertex(Triangles[i][2])
			.SetNormalAndTangent(Normal, Tangent)
			.SetColor(FColor::Blue)
			.SetTexCoord(FVector2f(1.0f, 0.0f));

		Builder.AddTriangle(V0, V1, V2, 0);
	}

	double t2 = FPlatformTime::Seconds();
	double delta = t2 - t1;
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("StreamSet Time To Compute: %f"), delta));
}

void FMarchingCubesThread::ThreadedMakeMesh()
{
	double t1 = FPlatformTime::Seconds();
	const FRealtimeMeshSectionGroupKey GroupKey = FRealtimeMeshSectionGroupKey::Create(0, FName("TestTriangle"));

	const FRealtimeMeshSectionKey PolyGroup0SectionKey = FRealtimeMeshSectionKey::CreateForPolyGroup(GroupKey, 0);

	RMC->CreateSectionGroup(GroupKey, StreamSet);
	RMC->UpdateSectionConfig(PolyGroup0SectionKey, FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Static, 0), true);

	double t2 = FPlatformTime::Seconds();
	double delta = t2 - t1;
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Mesh Time To Compute: %f"), delta));
}

