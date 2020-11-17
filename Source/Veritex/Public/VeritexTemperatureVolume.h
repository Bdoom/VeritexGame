// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "Components/SphereComponent.h"
#include "VeritexTemperatureVolume.generated.h"

/**
 * This class intent is to handle character temperature, it is a simple volume with a float which defines the temperature to be used in this volume. 
 * Higher Temperature (Heat) will cause players to get thirsty at a faster rate than average
 * Colder Temperature (Hypothermia) will cause the player to get colder, and use more food, also will cause user to get hypothermia if they are below a specific Fahrenheit.
 * The volume should also choose a random temperature between a range every 5 minutes which will set the temperature to a new value on all players within this volume.
 */
UCLASS()
class VERITEX_API AVeritexTemperatureVolume : public AActor
{
	GENERATED_BODY()

public:
	AVeritexTemperatureVolume();

	UPROPERTY(EditAnywhere)
		USphereComponent* SphereCollision;
	
	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, Category = "Temperature")
		float Temperature = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Temperature")
		int32 TemperatureMax = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Temperature")
		int32 TemperatureMinimum = 0;


	// Increase or decrease temperature by 30%
	UPROPERTY()
		float TemperatureModifier = 0.30f;

	virtual void BeginPlay() override;

	UFUNCTION()
	void ChooseNewTemperature();

	UFUNCTION()
		AVeritexWorldSettings* GetWorldSettings();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;



};
