// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GAM415_ReynoldsProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;

UCLASS(config=Game)
class AGAM415_ReynoldsProjectile : public AActor
{
	GENERATED_BODY()

	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* ballMesh;

	UPROPERTY(EditAnywhere)
		UMaterial* baseMat;

	UPROPERTY()
		FLinearColor randColor;

	UPROPERTY(EditAnywhere)
		UMaterialInterface* projMat;

	UPROPERTY()
		UMaterialInstanceDynamic* dmiMat;

	UPROPERTY(EditAnywhere)
		UNiagaraSystem* colorP;

// Moved timing to right as the projectile is spawned to allow us access to decal variables
public:
	AGAM415_ReynoldsProjectile();

protected:
	virtual void BeginPlay();


public:
	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Returns CollisionComp subobject **/
	USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
};

