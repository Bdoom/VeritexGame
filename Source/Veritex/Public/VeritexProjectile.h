// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "VeritexProjectile.generated.h"

UCLASS()
class VERITEX_API AVeritexProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVeritexProjectile();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UProjectileMovementComponent* Projectile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USphereComponent* Collider;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
		float BaseDamage = 10;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
