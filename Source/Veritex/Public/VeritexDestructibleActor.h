// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestructibleComponent.h"
#include "DestructibleMesh.h"
#include "Sound/SoundCue.h"
#include "VeritexDestructibleActor.generated.h"

UCLASS()
class VERITEX_API AVeritexDestructibleActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVeritexDestructibleActor();

	UPROPERTY()
		USceneComponent* Root;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		UDestructibleComponent* DM_Component;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		USoundCue* SoundToPlay;

	UFUNCTION(NetMulticast, Unreliable, WithValidation)
		void Multicast_ApplyDestructibleDamage();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
