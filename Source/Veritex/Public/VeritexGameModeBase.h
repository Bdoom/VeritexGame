// Daniel Gleason (C) 2017

#pragma once

#include "GameFramework/GameMode.h"
#include "VeritexGameModeBase.generated.h"

/**
 *
 */
UCLASS()
class VERITEX_API AVeritexGameModeBase : public AGameMode
{
	GENERATED_BODY()

public:
	AVeritexGameModeBase();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
		USoundBase* LoadingScreenSound;

};
