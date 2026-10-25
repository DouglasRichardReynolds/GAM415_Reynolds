// Fill out your copyright notice in the Description page of Project Settings.


#include "Portal.h"
#include "GAM415_ReynoldsCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

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

			// find the player's position relative to the portal.
			FVector LocalPlayerPosition = GetActorTransform().InverseTransformPosition(playerChar->GetActorLocation()
				);

			// determine which face of the portal was entered.
			bool bEnteredFront = LocalPlayerPosition.X >= 0.0f;

			// are we not teleporting
			if (!playerChar->isTeleporting)
			{
				// we are now teleporting, get other portal's location, move playerChar to stored location
				playerChar->isTeleporting = true;

				// get the destination portal's position and forward direction.
				FVector PortalLocation = OtherPortal->GetActorLocation();
				FVector PortalForward = OtherPortal->GetActorForwardVector();

				// choose the exit direction based on the entrance side.
				// BACK retains the previously working negative direction.
				// FRONT exits through the positive direction.
				FVector ExitDirection = bEnteredFront ? PortalForward : -PortalForward;

				// place the player outside the destination portal.
				FVector ExitLocation = PortalLocation + (ExitDirection * 200.0f);

				// face the player along the floor portal's placement direction.
				FVector FacingDirection = ExitDirection;

				if (FMath::Abs(ExitDirection.Z) > 0.9f)
				{
					FacingDirection = -OtherPortal->GetActorUpVector();

					if (!bEnteredFront)
					{
						FacingDirection *= -1.0f;
					}
				}

				// keep the player's view horizontal after teleporting.
				FacingDirection.Z = 0.0f;
				FacingDirection.Normalize();

				FRotator ExitRotation = FacingDirection.Rotation();
				ExitRotation.Pitch = 0.0f;
				ExitRotation.Roll = 0.0f;


				// rotate the player and their camera to face horizontally.
				playerChar->SetActorRotation(ExitRotation);


				// move the player to the destination portal.
				playerChar->SetActorLocationAndRotation(
					ExitLocation,
					ExitRotation,
					false,
					nullptr,
					ETeleportType::TeleportPhysics
				);

				// match the camera direction to the player's exit rotation.
				if (AController* PlayerController = playerChar->GetController())
				{
					PlayerController->SetControlRotation(ExitRotation);
				}

				// reset teleportation independently of the portal actor.
				TWeakObjectPtr<AGAM415_ReynoldsCharacter> WeakPlayer = playerChar;

				FTimerHandle TimerHandle;

				FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda(
					[WeakPlayer]()
					{
						if (WeakPlayer.IsValid())
						{
							WeakPlayer->isTeleporting = false;
						}
					}
				);

				GetWorld()->GetTimerManager().SetTimer(
					TimerHandle,
					TimerDelegate,
					1.0f,
					false
				);

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
	if (!IsValid(OtherPortal) || !IsValid(sceneCapture))
	{
		return;
	}

	// Get the player's camera.
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);

	if (!CameraManager)
	{
		return;
	}

	const FTransform CameraTransform = CameraManager->GetTransform();

	// calculate the camera position relative to this portal.
	const FTransform RelativeTransform = CameraTransform.GetRelativeTransform(GetActorTransform());

	// rotate the view to face out of the linked portal.
	const FTransform PortalRotation(FRotator(0.0f, 180.0f, 0.0f));

	const FTransform NewCameraTransform = RelativeTransform * PortalRotation * OtherPortal->GetActorTransform();

	// keep the capture camera outside the destination surface.
	const FVector PortalForward = OtherPortal->GetActorForwardVector();

	// position the capture camera in front of the destination portal.
	const FVector CaptureLocation = OtherPortal->GetActorLocation() + OtherPortal->GetActorForwardVector() * CaptureClearance;

	sceneCapture->SetWorldLocationAndRotation(
		CaptureLocation,
		NewCameraTransform.GetRotation()
	);

	// render the full scene without clipping geometry.
	sceneCapture->bEnableClipPlane = false;
}
