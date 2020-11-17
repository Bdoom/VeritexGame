// Daniel Gleason (C) 2017

#pragma once

#include "GameFramework/GameState.h"
#include "Structure.h"
#include "VeritexWorldSettings.h"
#include "VeritexGameState.generated.h"

UCLASS()
class VERITEX_API AVeritexGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "World Settings")
		AVeritexWorldSettings* GetWorldSettings();

	UPROPERTY(Replicated, BlueprintReadWrite)
		EWeatherMode WeatherMode;

	void ChooseWeather();
	
protected:

	virtual void BeginPlay() override;


};
