// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "TP_WeaponComponent.generated.h"

class AGAM415_ReynoldsCharacter;

// allows gun to reference portal class
class APortal;

class UTextureRenderTarget2D;
class UMaterialInterface;

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAM415_REYNOLDS_API UTP_WeaponComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AGAM415_ReynoldsProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;
	
	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector MuzzleOffset;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* FireMappingContext;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* FireAction;

	/** Sets default values for this component's properties */
	UTP_WeaponComponent();

	/** Attaches the actor to a FirstPersonCharacter */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	bool AttachWeapon(AGAM415_ReynoldsCharacter* TargetCharacter);

	/** Make the weapon Fire a Projectile */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void Fire();

	// allows Portal_BP to be assigned in the Unreal Editor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portal")
	TSubclassOf<APortal> PortalClass;

	// render targets used by the two active portal cameras.
	UPROPERTY(EditDefaultsOnly, Category = "Portal")
	TObjectPtr<UTextureRenderTarget2D> PortalRenderTarget1;

	UPROPERTY(EditDefaultsOnly, Category = "Portal")
	TObjectPtr<UTextureRenderTarget2D> PortalRenderTarget2;

	// materials used to display each portal's camera feed.
	UPROPERTY(EditDefaultsOnly, Category = "Portal")
	TObjectPtr<UMaterialInterface> PortalMaterial1;

	UPROPERTY(EditDefaultsOnly, Category = "Portal")
	TObjectPtr<UMaterialInterface> PortalMaterial2;

protected:
	/** Ends gameplay for this component. */
	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** The Character holding this weapon*/
	AGAM415_ReynoldsCharacter* Character;

	// stores portals in placement order, with the oldest portal at index 0.
	UPROPERTY()
	TArray<TObjectPtr<APortal>> ActivePortals;
};
