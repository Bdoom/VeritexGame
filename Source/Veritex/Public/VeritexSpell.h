// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VeritexSpell.generated.h"

class UParticleSystemComponent;
class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class VERITEX_API AVeritexSpell : public AActor
{
	GENERATED_BODY()
	
public:	

	// Sets default values for this actor's properties
	AVeritexSpell();

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = Spells)
		UParticleSystemComponent* ParticleSystemComponent;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = Spells)
		USphereComponent* SphereComponent;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = Spells)
		UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(BlueprintReadWrite, Replicated, EditAnywhere, Category = Spells)
		float BaseDamage;

	UFUNCTION()
		void OnComponentBeginOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = Spells)
		class UParticleSystem* Blood_PS;

	UFUNCTION(NetMulticast, Unreliable, WithValidation, Category = Spells)
		void Multicast_SpawnBloodParticle(AActor* OtherActor);

	UPROPERTY(Replicated, BlueprintReadWrite, Category = Spells)
		bool HasOverlappingActor;

	// Used for if we want consistent damage for a certain amount of time. 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Spells)
		float ConsistentDamageTimer = 3.f;

	UFUNCTION()
		void DealSpellDamage();

	// Used to determine if we are using consistent (Damage over time (DOT)) or if we are doing an overlap event.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Spells)
		bool ConsistentDamage;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
