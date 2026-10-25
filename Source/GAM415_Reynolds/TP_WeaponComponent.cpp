// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_WeaponComponent.h"
#include "GAM415_ReynoldsCharacter.h"
#include "GAM415_ReynoldsProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Engine/EngineTypes.h"
#include "Portal.h"
#include "Math/RotationMatrix.h"

// Sets default values for this component's properties
UTP_WeaponComponent::UTP_WeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
}

void UTP_WeaponComponent::Fire()
{
    if (Character == nullptr || Character->GetController() == nullptr)
    {
        return;
    }

    UWorld* World = GetWorld();

    if (World == nullptr)
    {
        return;
    }

    APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());

    if (PlayerController == nullptr ||
        PlayerController->PlayerCameraManager == nullptr)
    {
        return;
    }

    // get the player's camera position and direction.
    FVector Start =
        PlayerController->PlayerCameraManager->GetCameraLocation();

    FVector Forward =
        PlayerController->PlayerCameraManager->GetCameraRotation().Vector();

    // trace 5000 Unreal units forward from the camera.
    FVector End = Start + (Forward * 5000.0f);

    FHitResult HitResult;

    // prevent the trace from hitting the player.
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(Character);

    // check for a blocking surface along the trace.
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        TraceParams
    );

    if (bHit && PortalClass != nullptr)
    {
        // use the surface normal to orient the portal so its forward direction points outward from the wall.
        const FVector PortalForward = HitResult.ImpactNormal;

        // position the portal slightly away from the wall to reduce the chance of its mesh clipping into the surface.
        const FVector PortalLocation = HitResult.ImpactPoint + (PortalForward * 30.0f);

        // keep wall portals upright and align floor portals with the player's view.
        FVector PortalUp = FVector::UpVector;

        if (FMath::Abs(PortalForward.Z) > 0.9f)
        {
            PortalUp = FVector::VectorPlaneProject(
                Forward,
                PortalForward
            ).GetSafeNormal();
        }
        else
        {
            PortalUp = FVector::VectorPlaneProject(
                FVector::UpVector,
                PortalForward
            ).GetSafeNormal();
        }

        // align the portal's X-axis with the surface normal.
        const FRotator PortalRotation = FRotationMatrix::MakeFromXZ(PortalForward, PortalUp).Rotator();


        // allow the portal to spawn even when its overlap volume intersects the surface on which it is being placed.
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        // create an instance of the existing portal Blueprint. Linking portals will be implemented after placement works.
        APortal* NewPortal = World->SpawnActor<APortal>(
            PortalClass,
            PortalLocation,
            PortalRotation,
            SpawnParams
        );


        if (IsValid(NewPortal))
        {
            // remove the oldest portal when two already exist.
            if (ActivePortals.Num() >= 2)
            {
                APortal* OldestPortal = ActivePortals[0];

                ActivePortals.RemoveAt(0);

                if (IsValid(OldestPortal))
                {
                    OldestPortal->Destroy();
                }
            }

            // store the new portal in placement order.
            ActivePortals.Add(NewPortal);

            // configure both active portals when a pair exists.
            if (ActivePortals.Num() == 2)
            {
                APortal* FirstPortal = ActivePortals[0];
                APortal* SecondPortal = ActivePortals[1];

                if (IsValid(FirstPortal) && IsValid(SecondPortal))
                {
                    // link the portals for teleportation.
                    FirstPortal->OtherPortal = SecondPortal;
                    SecondPortal->OtherPortal = FirstPortal;

                    // give each portal its own camera render target.
                    FirstPortal->renderTarget = PortalRenderTarget1;
                    SecondPortal->renderTarget = PortalRenderTarget2;

                    FirstPortal->sceneCapture->TextureTarget = PortalRenderTarget1;
                    SecondPortal->sceneCapture->TextureTarget = PortalRenderTarget2;

                    // display each portal's own scene capture.
                    FirstPortal->mat = PortalMaterial1;
                    SecondPortal->mat = PortalMaterial2;

                    FirstPortal->mesh->SetMaterial(0, PortalMaterial1);
                    SecondPortal->mesh->SetMaterial(0, PortalMaterial2);
                }
            }

        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Portal failed to spawn."));
        }

    }

    // preserve the existing firing sound.
    if (FireSound != nullptr)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            FireSound,
            Character->GetActorLocation()
        );
    }

    // preserve the first-person firing animation.
    if (FireAnimation != nullptr)
    {
        UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();

        if (AnimInstance != nullptr)
        {
            AnimInstance->Montage_Play(FireAnimation, 1.f);
        }
    }
}

bool UTP_WeaponComponent::AttachWeapon(AGAM415_ReynoldsCharacter* TargetCharacter)
{
	Character = TargetCharacter;

	// check that the character is valid, and has no weapon component yet
	if (Character == nullptr || Character->GetInstanceComponents().FindItemByClass<UTP_WeaponComponent>())
	{
		return false;
	}

	// attach the weapon to the First Person Character
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));

	// add the weapon as an instance component to the character
	Character->AddInstanceComponent(this);

	// set up action bindings
	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// set the priority of the mapping to 1, so that it overrides the Jump action with the Fire action when using touch input
			Subsystem->AddMappingContext(FireMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			// fire
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &UTP_WeaponComponent::Fire);
		}
	}

	return true;
}

void UTP_WeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Character == nullptr)
	{
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(FireMappingContext);
		}
	}
}