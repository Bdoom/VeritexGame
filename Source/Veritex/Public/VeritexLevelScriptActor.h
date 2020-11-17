// Daniel Gleason (C) 2017

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "VeritexLevelScriptActor.generated.h"

/**
 * 
 */
UCLASS()
class VERITEX_API AVeritexLevelScriptActor : public ALevelScriptActor
{
	GENERATED_BODY()

	AVeritexLevelScriptActor();

	UFUNCTION(BlueprintPure, Category = WorldSettings)
		AVeritexWorldSettings* GetWorldSettings();

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Skydome)
		AActor* SkyDome;
	
	
};
