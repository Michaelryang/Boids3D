// Fill out your copyright notice in the Description page of Project Settings.


#include "MarchingCubesComponent.h"

UMarchingCubesComponent::UMarchingCubesComponent()
{

}

void UMarchingCubesComponent::BeginPlay()
{
	Super::BeginPlay();
	RealtimeMesh = InitializeRealtimeMesh<URealtimeMeshSimple>();

	RealtimeMesh->SetupMaterialSlot(0, "PrimaryMaterial", MarchingCubesMaterial);
	RealtimeMesh->SetupMaterialSlot(1, "SecondaryMaterial", MarchingCubesMaterial);
	//MaxCornerWorldSpace = MinCornerWorldSpace + FVector::One() * CubeSize * NumCubes;


}

FVector UMarchingCubesComponent::GetPositionFromIndices(int x, int y, int z)
{
	return FVector(MinCornerWorldSpace) + FVector(x, y, z) * CubeSize;
}


void UMarchingCubesComponent::ComputeVoxelGrid()
{
	double t1 = FPlatformTime::Seconds();
	VGrid = FVoxelGrid(
		int((MaxCornerWorldSpace.X - MinCornerWorldSpace.X) / CubeSize),
		int((MaxCornerWorldSpace.Y - MinCornerWorldSpace.Y) / CubeSize),
		int((MaxCornerWorldSpace.Z - MinCornerWorldSpace.Z) / CubeSize));
	float NoiseSample;

	for (float x = 0; x < VGrid.XResolution; ++x)
	{
		for (float y = 0; y < VGrid.YResolution; ++y)
		{
			for (float z = 0; z < VGrid.ZResolution; ++z)
			{
				FVector PositionWorldSpace = GetPositionFromIndices(x, y, z);
				NoiseSample = FMath::PerlinNoise3D(PositionWorldSpace / Scale);

				VGrid.Set((NoiseSample + 1.0) / 2.0, x, y, z);
			}
		}
	}

	double t2 = FPlatformTime::Seconds();
	double delta = t2 - t1;

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Voxel Time To Compute: %f"), delta));
}



void UMarchingCubesComponent::InitVoxelBounds(FVector BoundsMinCorner, FVector BoundsMaxCorner)
{
	MinCornerWorldSpace = FVector3f(BoundsMinCorner);
	MaxCornerWorldSpace = FVector3f(BoundsMaxCorner);
	ScaledBoxExtents = (MaxCornerWorldSpace - MinCornerWorldSpace) / 2;

	ComputeVoxelGrid();
}

FVector UMarchingCubesComponent::GetCornerPoint(int Corner)
{
	return FVector(CubePoints[Corner][0], CubePoints[Corner][1], CubePoints[Corner][2]);
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
				DrawDebugPoint(GetWorld(), GetPositionFromIndices(x, y, z), 10.0, SampleColor);
			}
		}
	}
	DrawDebugPoint(GetWorld(), FVector(MinCornerWorldSpace) - FVector::One(), 5.0, FColor::Green);
	DrawDebugPoint(GetWorld(), FVector(MaxCornerWorldSpace) + FVector::One(), 5.0, FColor::Blue);
}

bool UMarchingCubesComponent::PointIsOnContinuousBoundary(int x, int y, int z)
{
	return 
		(x == 0 && ContinuousBounds.XBack) ||
		(y == 0 && ContinuousBounds.YLeft) ||
		(z == 0 && ContinuousBounds.ZBottom) ||
		(x == VGrid.XResolution - 1 && ContinuousBounds.XFront) ||
		(y == VGrid.YResolution - 1 && ContinuousBounds.YRight) ||
		(z == VGrid.ZResolution - 1 && ContinuousBounds.ZTop);
}

void UMarchingCubesComponent::ComputeMarchingCubes()
{
	double t1 = FPlatformTime::Seconds();


	TArray<TArray<FVector3f>> Triangles;

	for (float x = 0; x < VGrid.XResolution - 1; ++x)
	{
		for (float y = 0; y < VGrid.YResolution - 1; ++y)
		{
			for (float z = 0; z < VGrid.ZResolution - 1; ++z)
			{
				int TriangulationIndex = 0;

				TArray<float> CubeData;

				for (int i = 0; i < 8; ++i)
				{
					if (PointIsOnContinuousBoundary(CubePoints[i][0] + x, CubePoints[i][1] + y, CubePoints[i][2] + z))
					{
						CubeData.Add(0.0);
					}
					else
					{
						CubeData.Add(VGrid.Get(CubePoints[i][0] + x, CubePoints[i][1] + y, CubePoints[i][2] + z));
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
						FVector3f VertexLocalPosition = VertexCubePosition - ScaledBoxExtents + FVector3f(x,y,z) * CubeSize;
						Triangles[Triangles.Num() - 1].Add(VertexLocalPosition);
					}
				}
			}
		}
	}
	
	FRealtimeMeshStreamSet StreamSet;
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

	const FRealtimeMeshSectionGroupKey GroupKey = FRealtimeMeshSectionGroupKey::Create(0, FName("TestTriangle"));

	const FRealtimeMeshSectionKey PolyGroup0SectionKey = FRealtimeMeshSectionKey::CreateForPolyGroup(GroupKey, 0);

	RealtimeMesh->CreateSectionGroup(GroupKey, StreamSet);
	RealtimeMesh->UpdateSectionConfig(PolyGroup0SectionKey, FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Static, 0), true);
	
	double t2 = FPlatformTime::Seconds();
	double delta = t2 - t1;

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Mesh Time To Compute: %f"), delta));
}