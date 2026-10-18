// Fill out your copyright notice in the Description page of Project Settings.


#include "Portal.h"
#include "GAM415_ReynoldsCharacter.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APortal::APortal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// creating default subobjects
	mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	boxComp = CreateDefaultSubobject<UBoxComponent>("Box Comp");
	sceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>("Capture");

	// setting attachments
	RootComponent = boxComp;
	mesh->SetupAttachment(boxComp);
	sceneCapture->SetupAttachment(mesh);

	// disabling mesh collision
	mesh->SetCollisionResponseToAllChannels(ECR_Ignore);

}

// Called when the game starts or when spawned
void APortal::BeginPlay()
{
	Super::BeginPlay();

	boxComp->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnOverlapBegin);
	mesh->SetHiddenInSceneCapture(true);

	if (mat) 
	{
		mesh->SetMaterial(0, mat);

	}
	
}

// Called every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdatePortals();

}

void APortal::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AGAM415_ReynoldsCharacter* playerChar = Cast<AGAM415_ReynoldsCharacter>(OtherActor);

	// is playerChar valid
	if (playerChar)
	{
		// is OtherPortal valid
		if (OtherPortal)
		{

			// Find the player's position relative to the portal.
			FVector LocalPlayerPosition =
				GetActorTransform().InverseTransformPosition(
					playerChar->GetActorLocation()
				);

			// Determine which face of the portal was entered.
			bool bEnteredFront = LocalPlayerPosition.X >= 0.0f;

			// Display the detected entrance side for testing.
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					3.0f,
					FColor::Yellow,
					bEnteredFront
					? TEXT("Entered FRONT of portal")
					: TEXT("Entered BACK of portal")
				);
			}


			// are we not teleporting
			if (!playerChar->isTeleporting)
			{
				// we are now teleporting, get other portal's location, move playerChar to stored location
				playerChar->isTeleporting = true;



				// Get the destination portal's position and forward direction.
				FVector PortalLocation = OtherPortal->GetActorLocation();
				FVector PortalForward = OtherPortal->GetActorForwardVector();

				// Choose the exit direction based on the entrance side.
				// BACK retains the previously working negative direction.
				// FRONT exits through the positive direction.
				FVector ExitDirection = bEnteredFront
					? PortalForward
					: -PortalForward;

				// Place the player outside the destination portal.
				FVector ExitLocation =
					PortalLocation + (ExitDirection * 200.0f);

				// Face the player away from the destination portal.
				FRotator ExitRotation = ExitDirection.Rotation();

				// Teleport the player and update their orientation.
				playerChar->SetActorLocation(ExitLocation);
				playerChar->SetActorRotation(ExitRotation);

				// Update the first-person camera direction.
				if (AController* PlayerController = playerChar->GetController())
				{
					PlayerController->SetControlRotation(ExitRotation);
				}


				// Update the controller rotation so the camera faces
				// the same direction as the character.
				if (AController* PlayerController = playerChar->GetController())
				{
					PlayerController->SetControlRotation(ExitRotation);
				}

				// wait 1 second, then change isTeleporting to false
				FTimerHandle TimerHandle;
				FTimerDelegate TimerDelegate;
				TimerDelegate.BindUFunction(this, "SetBool", playerChar);
				GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 1, false);
			}
		}
	}

}

void APortal::SetBool(AGAM415_ReynoldsCharacter* playerChar)
{
	if (playerChar)
	{
		playerChar->isTeleporting = false;

	}
}


void APortal::UpdatePortals()
{
	// Make sure both portals are linked before updating.
	if (!OtherPortal || !sceneCapture)
	{
		return;
	}

	// Get the player's current camera.
	APlayerCameraManager* CameraManager =
		UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);

	if (!CameraManager)
	{
		return;
	}

	// Get the player's camera transform.
	const FTransform CameraTransform =
		CameraManager->GetTransform();

	// Convert the camera position and rotation into
	// coordinates relative to this portal.
	const FTransform RelativeTransform =
		CameraTransform.GetRelativeTransform(GetActorTransform());

	// Rotate 180 degrees to account for looking through
	// the portal and emerging from the linked portal.
	const FTransform PortalRotation(
		FRotator(0.0f, 180.0f, 0.0f)
	);

	// Transform the camera into the linked portal's space.
	const FTransform NewCameraTransform =
		RelativeTransform * PortalRotation *
		OtherPortal->GetActorTransform();

	// Update the scene capture to show the correct view.
	sceneCapture->SetWorldLocationAndRotation(
		NewCameraTransform.GetLocation(),
		NewCameraTransform.GetRotation()
	);
}

