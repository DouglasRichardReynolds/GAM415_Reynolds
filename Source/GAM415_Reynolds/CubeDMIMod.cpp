// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeDMIMod.h"
#include "GAM415_ReynoldsCharacter.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ACubeDMIMod::ACubeDMIMod()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Defining Subobjects
	boxComp = CreateDefaultSubobject<UBoxComponent>("Box Component");
	cubeMesh = CreateDefaultSubobject<UStaticMeshComponent>("Cube Mesh");

	// Setting attachments
	RootComponent = boxComp;
	cubeMesh->SetupAttachment(boxComp);

}

// Called when the game starts or when spawned
void ACubeDMIMod::BeginPlay()
{
	Super::BeginPlay();

	boxComp->OnComponentBeginOverlap.AddDynamic(this, &ACubeDMIMod::OnOverLapBegin);

	// If baseMat is valid
	if (baseMat)
	{
		// Creating instance of material
		dmiMat = UMaterialInstanceDynamic::Create(baseMat, this);
	}

	// If cubeMesh is valid
	if (cubeMesh)
	{
		// Set material to dmiMat (instance)
		cubeMesh->SetMaterial(0, dmiMat);

	}
	
}

// Called every frame
void ACubeDMIMod::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACubeDMIMod::OnOverLapBegin(UPrimitiveComponent* OverLappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AGAM415_ReynoldsCharacter* overlappedActor = Cast<AGAM415_ReynoldsCharacter>(OtherActor);

	if (overlappedActor)
	{
		// Generating random color
		float ranNumX = UKismetMathLibrary::RandomFloatInRange(0.f, 1.f);
		float ranNumY = UKismetMathLibrary::RandomFloatInRange(0.f, 1.f);
		float ranNumZ = UKismetMathLibrary::RandomFloatInRange(0.f, 1.f);

		// Setting random color
		FVector4 randColor = FVector4(ranNumX, ranNumY, ranNumZ, 1.f);

		// If dmiMat is valid
		if (dmiMat)
		{
			// Apply random color and darkness
			dmiMat->SetVectorParameterValue("Color", randColor);
			dmiMat->SetScalarParameterValue("Darkness", ranNumX);
		}
	}

}

