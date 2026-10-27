
#include "Portal.h"
#include "GAM415_ReynoldsCharacter.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

APortal::APortal()
{
    PrimaryActorTick.bCanEverTick = true;

    // creating the portal components.
    boxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Comp"));
    mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    sceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Capture"));

    // setting attachments
    RootComponent = boxComp;
    mesh->SetupAttachment(boxComp);
    sceneCapture->SetupAttachment(mesh);

    // prevent the portal mesh from blocking movement.
    mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
}
// called when the game starts or when spawned
void APortal::BeginPlay()
{
    Super::BeginPlay();

    boxComp->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnOverlapBegin);

    // keep the portal surface out of scene captures.
    mesh->SetHiddenInSceneCapture(true);

    if (mat)
    {
        mesh->SetMaterial(0, mat);
    }
}

// called every frame
void APortal::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdatePortals();
}

void APortal::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AGAM415_ReynoldsCharacter* PlayerCharacter = Cast<AGAM415_ReynoldsCharacter>(OtherActor);

    // only teleport valid players with an active portal link.
    if (!IsValid(PlayerCharacter) || !IsValid(OtherPortal))
    {
        return;
    }

    // prevent repeated teleports during the cooldown.
    if (PlayerCharacter->isTeleporting)
    {
        return;
    }

    // determine which side of the portal the player entered.
    const FVector LocalPlayerPosition = GetActorTransform().InverseTransformPosition(PlayerCharacter->GetActorLocation());

    const bool bEnteredFront = LocalPlayerPosition.X >= 0.0f;

    PlayerCharacter->isTeleporting = true;

    const FVector PortalLocation = OtherPortal->GetActorLocation();
    const FVector PortalForward = OtherPortal->GetActorForwardVector();

    // select the exit side based on the entrance side.
    const FVector ExitDirection = bEnteredFront ? PortalForward : -PortalForward;

    // place the player outside the destination portal.
    const FVector ExitLocation = PortalLocation + (ExitDirection * 200.0f);

    // face the player along the floor portal's placement direction.
    FVector FacingDirection = ExitDirection;
    // Use the portal's orientation when exiting a floor portal.
    if (FMath::Abs(ExitDirection.Z) > 0.9f)
    {
        FacingDirection = -OtherPortal->GetActorUpVector();

        if (!bEnteredFront)
        {
            FacingDirection *= -1.0f;
        }
    }

    // keep the player's view horizontal after teleportation.
    FacingDirection.Z = 0.0f;
    FacingDirection.Normalize();

    FRotator ExitRotation = FacingDirection.Rotation();
    ExitRotation.Pitch = 0.0f;
    ExitRotation.Roll = 0.0f;

    // move the player and apply the exit orientation.
    PlayerCharacter->SetActorLocationAndRotation(ExitLocation, ExitRotation, false, nullptr, ETeleportType::TeleportPhysics);

    if (AController* PlayerController = PlayerCharacter->GetController())
    {
        PlayerController->SetControlRotation(ExitRotation);
    }

    // reset the cooldown without depending on the portal's lifetime.
    const TWeakObjectPtr<AGAM415_ReynoldsCharacter> WeakPlayer = PlayerCharacter;

    FTimerHandle TimerHandle;

    // reset teleportation state
    FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda([WeakPlayer]()
        {
            if (WeakPlayer.IsValid())
            {
                WeakPlayer->isTeleporting = false;
            }
        }
    );

    GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 1.0f, false);
}

void APortal::SetBool(AGAM415_ReynoldsCharacter* PlayerCharacter)
{
    if (IsValid(PlayerCharacter))
    {
        PlayerCharacter->isTeleporting = false;
    }
}

void APortal::UpdatePortals()
{
    if (!IsValid(OtherPortal) || !IsValid(sceneCapture))
    {
        return;
    }

    // get the player's camera
    APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);

    if (!IsValid(CameraManager))
    {
        return;
    }

    const FTransform CameraTransform = CameraManager->GetTransform();

    // calculate the player's camera orientation relative to this portal.
    const FTransform RelativeTransform = CameraTransform.GetRelativeTransform(GetActorTransform());

    // rotate the view to face out of the linked portal.
    const FTransform PortalRotation(FRotator(0.0f, 180.0f, 0.0f));

    const FTransform NewCameraTransform = RelativeTransform * PortalRotation * OtherPortal->GetActorTransform();

    // keep the capture at a fixed position outside the destination surface.
    const FVector CaptureLocation = OtherPortal->GetActorLocation() + OtherPortal->GetActorForwardVector() * CaptureClearance;

    sceneCapture->SetWorldLocationAndRotation(CaptureLocation, NewCameraTransform.GetRotation());

    // render the full scene without a clipping plane.
    sceneCapture->bEnableClipPlane = false;
}
