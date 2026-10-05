// Fill out your copyright notice in the Description page of Project Settings.


#include "PerlinProcTerrain.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"

// Sets default values
APerlinProcTerrain::APerlinProcTerrain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// creating subobject
	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>("Procedural Mesh");

	// setting up attachment
	ProcMesh->SetupAttachment(GetRootComponent());

}

// Called when the game starts or when spawned
void APerlinProcTerrain::BeginPlay()
{
	Super::BeginPlay();
	
	CreateVertices();
	CreateTraingles();

	ProcMesh->CreateMeshSection(sectionID, Vertices, Triangles, Normals, UV0, UpVertexColors, TArray<FProcMeshTangent>(), true);

	ProcMesh->SetMaterial(0, Mat);

}

// Called every frame
void APerlinProcTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APerlinProcTerrain::AlterMesh(FVector impactPoint)
{
	// each vertex in mesh
	for (int i = 0; i < Vertices.Num(); i++)
	{
		// making dig happen at ProcTerrain location
		FVector tempVector = impactPoint - this->GetActorLocation();

		// if hit vertice is within radius value
		if (FVector(Vertices[i] - tempVector).Size() < radius)
		{
			// moving vertice based on depth value
			Vertices[i] = Vertices[i] - Depth;

			// updating mesh with new vertice values
			ProcMesh->UpdateMeshSection(sectionID, Vertices, Normals, UV0, UpVertexColors, TArray<FProcMeshTangent>());
		}
	}

}

void APerlinProcTerrain::CreateVertices()
{
	// nested for loop
	for (int X = 0; X <= XSize; X++)
	{
		for (int Y = 0; Y <= YSize; Y++)
		{
			// using perlin noise to create a random scale
			float Z = FMath::PerlinNoise2D(FVector2D(X * NoiseScale + 0.1, Y * NoiseScale)) * ZMultiplier;

			// printing scale value
			GEngine->AddOnScreenDebugMessage(-1, 999.0f, FColor::Yellow, FString::Printf(TEXT("Z %f"), Z));

			// adding vertex using X and Y values, multiplied by perlin noise scale
			Vertices.Add(FVector(X * Scale, Y * Scale, Z));

			// adding UV's using X and Y values, with perlin noise added
			UV0.Add(FVector2D(X * UVScale, Y * UVScale));
		}
	}

}

void APerlinProcTerrain::CreateTraingles()
{
	int Vertex = 0;

	// nested for loop
	for (int X = 0; X < XSize; X++)
	{
		for (int Y = 0; Y < YSize; Y++)
		{
			// adding vertexes based on determined size
			Triangles.Add(Vertex);
			Triangles.Add(Vertex + 1);
			Triangles.Add(Vertex + YSize + 1);
			Triangles.Add(Vertex + 1);
			Triangles.Add(Vertex + YSize + 2);
			Triangles.Add(Vertex + YSize + 1);

			Vertex++;
		}

		Vertex++;
	}

}

