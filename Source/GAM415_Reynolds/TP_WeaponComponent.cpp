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

    APlayerController* PlayerController =
        Cast<APlayerController>(Character->GetController());

    if (PlayerController == nullptr ||
        PlayerController->PlayerCameraManager == nullptr)
    {
        return;
    }

    // Get the player's camera position and direction.
    FVector Start =
        PlayerController->PlayerCameraManager->GetCameraLocation();

    FVector Forward =
        PlayerController->PlayerCameraManager->GetCameraRotation().Vector();

    // Trace 5000 Unreal units forward from the camera.
    FVector End = Start + (Forward * 5000.0f);

    FHitResult HitResult;

    // Prevent the trace from hitting the player.
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(Character);

    // Check for a blocking surface along the trace.
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        TraceParams
    );

    // Display a green line when a surface is hit,
    // or a red line if the trace misses.
    DrawDebugLine(
        World,
        Start,
        bHit ? HitResult.ImpactPoint : End,
        bHit ? FColor::Green : FColor::Red,
        false,
        2.0f,
        0,
        2.0f
    );

    if (bHit)
    {
        // Mark the impact location for testing.
        DrawDebugSphere(
            World,
            HitResult.ImpactPoint,
            20.0f,
            12,
            FColor::Yellow,
            false,
            2.0f
        );
    }

    // Preserve the existing firing sound.
    if (FireSound != nullptr)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            FireSound,
            Character->GetActorLocation()
        );
    }

    // Preserve the first-person firing animation.
    if (FireAnimation != nullptr)
    {
        UAnimInstance* AnimInstance =
            Character->GetMesh1P()->GetAnimInstance();

        if (AnimInstance != nullptr)
        {
            AnimInstance->Montage_Play(FireAnimation, 1.f);
        }
    }
}

bool UTP_WeaponComponent::AttachWeapon(AGAM415_ReynoldsCharacter* TargetCharacter)
{
	Character = TargetCharacter;

	// Check that the character is valid, and has no weapon component yet
	if (Character == nullptr || Character->GetInstanceComponents().FindItemByClass<UTP_WeaponComponent>())
	{
		return false;
	}

	// Attach the weapon to the First Person Character
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));

	// add the weapon as an instance component to the character
	Character->AddInstanceComponent(this);

	// Set up action bindings
	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Set the priority of the mapping to 1, so that it overrides the Jump action with the Fire action when using touch input
			Subsystem->AddMappingContext(FireMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			// Fire
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